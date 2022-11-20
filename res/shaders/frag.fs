#version 410 core

out vec4 FragCol;

float FLT_MAX = 999999999.0;

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
        vec3 origin = ray.Origin - vec3(u_Spheres[i].Position.xyz);
        float a = dot(ray.Direction, ray.Direction);
        float b = 2.0 * dot(origin, ray.Direction);
        float c = dot(origin, origin) - u_Spheres[i].Radius * u_Spheres[i].Radius;

        // discriminant hit test
        float discriminant = b * b - 4.0 * a * c;

        if (discriminant < 0.0)
            continue;

        // float rootOne = (-b + discriminant) / (2.0 * a);
        float closestRoot = (-b - discriminant) / (2.0 * a);
        if (closestRoot < hitDistance)
        {
            hitDistance = closestRoot;
            closestSphereIndex = i;
        }
    }

    if (closestSphereIndex == -1)
        //return u_Spheres[0].Position;
        return mix(skyBlue, vec4(0.0902, 0.2784, 0.4784, 1.0), uv.y);
    
    vec3 origin = ray.Origin - vec3(u_Spheres[closestSphereIndex].Position.xyz);
    vec3 hitPoint = origin + ray.Direction * hitDistance;
    vec3 normal = normalize(hitPoint);

    vec3 LightDirection = normalize(u_LightDirection);
    float LightIntensity = max(dot(-LightDirection, normal), 0.0);
    vec4 SphereCol = u_Spheres[closestSphereIndex].Albedo;
    SphereCol *= LightIntensity;

    return SphereCol; 
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
