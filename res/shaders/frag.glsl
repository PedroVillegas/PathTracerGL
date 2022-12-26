#version 410 core

#define FLT_MAX 3.402823466e+38

out vec4 FragCol;

uniform vec3 u_LightDirection;
uniform vec2 u_Resolution;
uniform vec3 u_RayOrigin;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;

struct Material
{
    vec4 Albedo;
    //float Roughness;
    //float Metallic;
};

struct Sphere
{
    vec4 Position; // Radius stored in Position.w

    Material Mat;
};

layout (std140) uniform ObjectData
{
    int sphereCount;
    Sphere Spheres[2];
} objectData;

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
    
    Sphere closestSphere = objectData.Spheres[objectIndex];

    // Calculate outside normal of sphere and intersection point in world space
    vec3 centre = vec3(closestSphere.Position.xyz);
    payload.WorldPosition = ray.Origin + ray.Direction * payload.HitDistance;
    payload.WorldNormal = normalize(payload.WorldPosition - centre);

    return payload;
}

HitRecord Miss()
{
    HitRecord payload;
    payload.HitDistance = -1;
    return payload;
}

HitRecord TraceRay(Ray ray)
{
    int closestSphereIndex = -1;
    float hitDistance = FLT_MAX;

    for (int i = 0; i < objectData.sphereCount; i++)
    {        
        Sphere sphere = objectData.Spheres[i];
        vec3 origin = ray.Origin - sphere.Position.xyz;
        float radius = sphere.Position.w;

        // Evaluate intersections points between ray and sphere
        // by solving quadratic equation
        float a = dot(ray.Direction, ray.Direction);
        float half_b = dot(origin, ray.Direction);
        float c = dot(origin, origin) - radius * radius;

        // Discriminant hit test (< 0 means no real solution)
        float discriminant = half_b * half_b - a * c;
        if (discriminant < 0.0)
            continue;
        
        // Ignore furthest intersection
        // float rootOne = (-b + sqrt(discriminant)) / (2.0 * a);
        float closestRoot = (-half_b - sqrt(discriminant)) / a;
        if (closestRoot > 0.0 && closestRoot < hitDistance)
        {
            hitDistance = closestRoot;
            closestSphereIndex = i;
        }
    }

    if (closestSphereIndex < 0)
        return Miss();

    return ClosestHit(ray, hitDistance, closestSphereIndex);
}

vec4 PerPixel(vec2 uv)
{
    Ray ray;

    // Ray direction in world space
    vec4 target = u_InverseProjection * vec4(uv.xy, 1, 1);
    ray.Direction = vec3(u_InverseView * vec4(normalize(vec3(target.xyz) / target.w), 0));
    ray.Origin = u_RayOrigin;

    // Multiplier simplifies more bounces -> lower contribution to final colour 
    vec3 colour = vec3(0.0);
    float multiplier = 1.0;

    int depth = 16;
    for (int i = 0; i < depth; i++)
    {
        // Keep track of ray intersection point, direction etc
        HitRecord payload = TraceRay(ray);

        // If ray misses, object takes on colour of the sky
        if (payload.HitDistance < 0)
        {
            vec3 skyColour = vec3(0.3, 0.6, 0.8);
            colour += skyColour * multiplier;
            break;
        }

        vec3 lightDir = normalize(u_LightDirection);
        float lightIntensity = max(dot(payload.WorldNormal, -lightDir), 0);

        // Closest object to the eye -> contributes the most colour
        Sphere sphere = objectData.Spheres[payload.ObjectIndex];
        vec3 sphereColour = vec3(sphere.Mat.Albedo);
        sphereColour *= lightIntensity;
        colour += sphereColour * multiplier;

        multiplier *= 0.5;

        // Prevent z-fighting situation and reflects ray for the path
        ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001;
        ray.Direction = reflect(ray.Direction, payload.WorldNormal);
    }

    return vec4(colour, 1.0); // vec4(payload.WorldNormal * 0.5 + 0.5, 1);
}

void main()
{
    // Pixel coord in NDC [-1, 1]
    vec2 uv = gl_FragCoord.xy / u_Resolution.xy;
    uv = (uv * 2.0) - 1.0;

    FragCol = PerPixel(uv);
}
