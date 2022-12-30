#version 410 core

#define FLT_MAX 3.402823466e+38
#define PI 3.14159

out vec4 FragCol;

uniform int u_Depth;
uniform int u_FrameCounter;
uniform vec3 u_LightDirection;
uniform vec2 u_Resolution;
uniform vec3 u_RayOrigin;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;
uniform sampler2D u_PreviousFrame;

struct Material
{
    vec4 albedo;
    float roughness;
    //float Metallic;
};

struct Sphere
{
    vec4 position; // Radius stored in position.w

    Material mat;
};

layout (std140) uniform ObjectData
{
    int sphereCount;
    Sphere Spheres[2];
} objectData;

struct Ray
{
    vec3 origin;
    vec3 direction;
};

struct HitRecord
{
    float hitDistance;
    vec3 worldPosition;
    vec3 worldNormal;

    int objectIndex;
};

uint PCGHash(uint seed)
{
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float Rand(uint seed, float min, float max)
{
    float base = float(PCGHash(seed)) / 4294967295.0;
    return min + base * (max - min);
}

vec3 RandomSampleUnitSphere(uint seed)
{
    float theta = Rand(seed, 0, 2 * PI);
    float z = Rand(seed, -1.0, 1.0);
    float r = sqrt(max(0.0, 1.0 - z * z));
    float x = r * cos(theta);
    float y = r * sin(theta);
    vec3 result = vec3(x, y, z);
    result *= pow(Rand(seed, 0.0, 1.0), 1.0 / 3.0);
    return result;
}

HitRecord ClosestHit(Ray ray, float hitDistance, int objectIndex)
{
    HitRecord payload;
    payload.hitDistance = hitDistance;
    payload.objectIndex = objectIndex;
    
    Sphere closestSphere = objectData.Spheres[objectIndex];

    // Calculate outside normal of sphere and intersection point in world space
    vec3 centre = vec3(closestSphere.position.xyz);
    payload.worldPosition = ray.origin + ray.direction * hitDistance;
    payload.worldNormal = normalize(payload.worldPosition - centre);

    return payload;
}

HitRecord Miss()
{
    HitRecord payload;
    payload.hitDistance = -1;
    return payload;
}

HitRecord TraceRay(Ray ray)
{
    int closestSphereIndex = -1;
    float hitDistance = FLT_MAX;

    for (int i = 0; i < objectData.sphereCount; i++)
    {        
        Sphere sphere = objectData.Spheres[i];
        vec3 origin = ray.origin - sphere.position.xyz;
        float radius = sphere.position.w;

        // Evaluate intersections points between ray and sphere
        // by solving quadratic equation
        float a = dot(ray.direction, ray.direction);
        float half_b = dot(origin, ray.direction);
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

vec4 PerPixel(vec2 uv, uint seed)
{
    Ray ray;

    // Ray direction in world space
    vec4 target = u_InverseProjection * vec4(uv.xy, 1, 1);
    ray.direction = vec3(u_InverseView * vec4(normalize(vec3(target.xyz) / target.w), 0));
    ray.origin = u_RayOrigin;

    // Multiplier simplifies more bounces -> lower contribution to final colour 
    vec3 colour = vec3(0.0);
    float multiplier = 1.0;

    int depth = 4;
    for (int i = 0; i < depth; i++)
    {
        // Keep track of ray intersection point, direction etc
        HitRecord payload = TraceRay(ray);

        // If ray misses, object takes on colour of the sky
        if (payload.hitDistance < 0)
        {
            vec3 skyColour = vec3(0.5, 0.7, 1.0);// mix(vec3(1.0), vec3(0.5, 0.7, 1.0), uv.y);
            colour += skyColour * multiplier;
            break;
        }

        vec3 lightDir = normalize(u_LightDirection);
        float lightIntensity = max(dot(payload.worldNormal, -lightDir), 0);

        // Closest object to the eye -> contributes the most colour
        Sphere sphere = objectData.Spheres[payload.objectIndex];
        vec3 sphereColour = vec3(sphere.mat.albedo);
        sphereColour *= lightIntensity;
        colour += sphereColour * multiplier;

        multiplier *= 0.5;

        // Prevent z-fighting situation and reflects ray for the path
        ray.origin = payload.worldPosition + payload.worldNormal * 0.0001;
        vec3 new_dir = reflect(ray.direction, payload.worldNormal);
        ray.direction = normalize(new_dir + sphere.mat.roughness * RandomSampleUnitSphere(seed));
        // ray.direction = normalize(reflect(ray.direction, 
        //     payload.worldNormal + sphere.mat.roughness * RandomSampleUnitSphere(seed)));
    }

    return vec4(colour, 1.0); // vec4(payload.worldNormal * 0.5 + 0.5, 1);
}

void main()
{
    // Pixel coord in NDC [-1, 1]
    vec2 uv = gl_FragCoord.xy / u_Resolution.xy;
    uv = (uv * 2.0) - 1.0;

    uint seed = uint(gl_FragCoord.x * 1973 + gl_FragCoord.y * 9277 + u_FrameCounter * 2699) | uint(1);
    vec4 finalColour = PerPixel(uv, seed);
    //vec4 prevFrameColour = texture(u_PreviousFrame, uv);
    FragCol = finalColour;//(finalColour + prevFrameColour)/2;
}
