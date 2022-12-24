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

    vec3 colour = vec3(0.0);
    float multiplier = 1.0;

    int depth = 10;
    for (int i = 0; i < depth; i++)
    {
        HitRecord payload = TraceRay(ray);

        if (payload.HitDistance < 0)
        {
            vec3 skyColour = vec3(0.3, 0.6, 0.8);
            colour += skyColour * multiplier;
            break;
        }

        vec3 lightDir = normalize(u_LightDirection);
        float lightIntensity = max(dot(payload.WorldNormal, -lightDir), 0);

        Sphere sphere = u_Spheres[payload.ObjectIndex];
        vec3 sphereColour = vec3(sphere.Albedo);//vec3(1.0);
        sphereColour *= lightIntensity;
        colour += sphereColour * multiplier;

        multiplier *= 0.5;

        ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001;
        ray.Direction = reflect(ray.Direction, payload.WorldNormal);
    }

    return vec4(colour, 1.0); // vec4(payload.WorldNormal * 0.5 + 0.5, 1);
}

void main()
{
    // pixel coord in NDC
    vec2 uv = gl_FragCoord.xy / u_Resolution.xy;
    uv = (uv * 2.0) - 1.0;

    FragCol = PerPixel(uv);
}
