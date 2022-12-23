#version 410 core

#define FLT_MAX 3.402823466e+38

out vec4 FragCol;

uniform vec3 u_LightDirection;
uniform vec2 u_Resolution;
uniform vec3 u_RayOrigin;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;
uniform int u_SphereCount;

struct Sphere
{
    vec4 Position;
    float Radius;
    vec4 Albedo;
};

layout (std140) uniform SpheresBlock
{
    Sphere u_Spheres[2];
};

struct Ray
{
    vec3 Origin;
    vec3 Direction;
};

struct HitRecord
{
    float HitDistance;
    vec3 WorldPosition;
    vec3 WorldNormal;

    int ObjectIndex;
};

vec4 skyBlue = vec4(0.2549, 0.5412, 0.8471, 1.0);

HitRecord ClosestHit(Ray ray, float hitDistance, int objectIndex)
{
    HitRecord payload;
    payload.HitDistance = hitDistance;
    payload.ObjectIndex = objectIndex;
    

    Sphere closestSphere = u_Spheres[objectIndex];

    // Calculate outside normal of sphere and intersection point in world space
    vec3 centre = vec3(closestSphere.Position.xyz);
    payload.WorldPosition = ray.Origin + ray.Direction * payload.HitDistance;
    payload.WorldNormal = normalize(payload.WorldPosition - centre);

    return payload;
}

HitRecord Miss(Ray ray)
{
    HitRecord payload;
    payload.HitDistance = -1;
    return payload;
}

HitRecord TraceRay(Ray ray)
{
    int closestSphereIndex = -1;
    float hitDistance = FLT_MAX;

    for (int i = 0; i < u_SphereCount; i++)
    {        
        Sphere sphere = u_Spheres[i];
        vec3 origin = ray.Origin - vec3(sphere.Position.xyz);

        float a = dot(ray.Direction, ray.Direction);
        float half_b = dot(origin, ray.Direction);
        float c = dot(origin, origin) - sphere.Radius * sphere.Radius;

        // discriminant hit test
        float discriminant = half_b * half_b - a * c;
        if (discriminant < 0.0)
            continue;

        // float rootOne = (-b + sqrt(discriminant)) / (2.0 * a);
        float closestRoot = (-half_b - sqrt(discriminant)) / a;
        if (closestRoot > 0.0 && closestRoot < hitDistance)
        {
            hitDistance = closestRoot;
            closestSphereIndex = i;
        }
    }

    if (closestSphereIndex < 0)
        return Miss(ray);

    return ClosestHit(ray, hitDistance, closestSphereIndex);
}

vec4 PerPixel(vec2 uv)
{
    Ray ray;

    vec4 target = u_InverseProjection * vec4(uv.xy, 1, 1);
    ray.Direction = vec3(u_InverseView * vec4(normalize(vec3(target.xyz) / target.w), 0)); // Ray direction in world space
    ray.Origin = u_RayOrigin;

    HitRecord payload = TraceRay(ray);

    if (payload.HitDistance < 0)
        return mix(skyBlue, vec4(0.0902, 0.2784, 0.4784, 1.0), uv.y);

    return vec4(payload.WorldNormal * 0.5 + 0.5, 1);
}

void main()
{
    // pixel coord in NDC
    vec2 uv = gl_FragCoord.xy / u_Resolution.xy;
    uv = (uv * 2.0) - 1.0;

    FragCol = PerPixel(uv);
}
