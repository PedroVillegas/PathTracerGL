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

vec4 skyBlue = vec4(0.2549, 0.5412, 0.8471, 1.0);

vec4 TraceRay(Ray ray, vec2 uv)
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
        return mix(skyBlue, vec4(0.0902, 0.2784, 0.4784, 1.0), uv.y);
    
    // Calculate outside normal of sphere
    vec3 centre = vec3(u_Spheres[closestSphereIndex].Position.xyz);
    vec3 hitPoint = ray.Origin + ray.Direction * hitDistance;
    vec3 normal = normalize(hitPoint - centre);

    return vec4(normal * 0.5 + 0.5, 1.0); 
}

void main()
{
    // pixel coord in NDC
    vec2 uv = gl_FragCoord.xy / u_Resolution.xy;
    uv = (uv * 2.0) - 1.0;

    Ray ray;

    vec4 target = u_InverseProjection * vec4(uv.xy, 1, 1);
    ray.Direction = vec3(u_InverseView * vec4(normalize(vec3(target.xyz) / target.w), 0)); // Ray direction in world space
    ray.Origin = u_RayOrigin;

    FragCol = TraceRay(ray, uv);
}
