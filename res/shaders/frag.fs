#version 410 core

in vec4 vertCol;
out vec4 FragCol;

uniform vec2 u_Resolution;

uniform vec3 u_RayOrigin;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;

uniform vec4 u_SphereCol;

uniform vec3 u_LightDirection;

struct Ray
{
    vec3 Origin;
    vec3 Direction;
};

void main()
{
    // pixel coord in NDC
    vec2 uv = gl_FragCoord.xy / u_Resolution.xy;
    uv = (uv * 2.0) - 1.0;

    Ray ray;

    vec4 target = u_InverseProjection * vec4(uv.xy, 1, 1);
    ray.Direction = vec3(u_InverseView * vec4(normalize(vec3(target.xyz) / target.w), 0)); // Ray direction in world space
    ray.Origin = u_RayOrigin;
    float radius = 0.5;

    float a = dot(ray.Direction, ray.Direction);
    float b = 2.0 * dot(ray.Origin, ray.Direction);
    float c = dot(ray.Origin, ray.Origin) - radius * radius;

    // discriminant hit test
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) 
        FragCol = (1 - (uv.y * 0.5 + 0.5)) * vec4(1.0, 1.0, 1.0, 1.0) + (uv.y * 0.5 + 0.5) * vec4(0.5, 0.7, 1.0, 1.0); // vec4(0.45, 0.55, 0.6, 1.0);
    else 
    {
        // float rootOne = (-b + discriminant) / (2.0 * a);
        float closestRoot = (-b - discriminant) / (2.0 * a);

        if (closestRoot < 0.0)
            FragCol = vec4(0.0, 0.0, 0.0, 1.0);
        else
        {
            vec3 hitPoint = ray.Origin + ray.Direction * closestRoot;
            vec3 normal = normalize(hitPoint);

            vec3 LightDirection = normalize(u_LightDirection);
            float LightIntensity = max(dot(-LightDirection, normal), 0.0);
            vec4 SphereCol = u_SphereCol;
            SphereCol *= LightIntensity;

            FragCol = SphereCol;
        }
    }    
}
