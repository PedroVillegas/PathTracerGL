#version 410 core

in vec4 vertCol;
out vec4 colour;

uniform vec2 u_resolution;
uniform vec4 u_SphereCol;

void main()
{
    // pixel coord in NDC space
    vec2 uv = gl_FragCoord.xy / u_resolution.xy;
    uv = (uv * 2.0) - 1.0;

    vec3 rayOrigin = vec3(0.0, 0.0, 1.0);
    vec3 rayDir = normalize(vec3(uv.x, uv.y, -1.0));
    float radius = 0.5;

    vec3 lightDirection = normalize(vec3(-1, -1, -1));

    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(rayOrigin, rayDir);
    float c = dot(rayOrigin, rayOrigin) - radius * radius;

    // discriminant hit test
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) 
    {
        colour = vec4(0.0, 0.0, 0.0, 1.0);
    } 
    else 
    {
        float t0 = (-b + discriminant) / (2 * a);
        float closestT = (-b - discriminant) / (2 * a);

        vec3 hitPoint = rayOrigin + rayDir * closestT;
        vec3 normal = normalize(hitPoint);
        vec4 SphereCol = u_SphereCol;
        float d = max(dot(-lightDirection, normal), 0);
        SphereCol *= d;

        colour = SphereCol;
    }    
}
