#version 410 core

#define FLT_MAX 3.402823466e+38
#define PI 3.14159

out vec4 colour;

uniform int u_SampleIterations;
uniform int u_SamplesPerPixel;
uniform int u_Depth;
uniform vec2 u_Resolution;
uniform vec3 u_RayOrigin;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;
uniform sampler2D u_AccumulationTexture;

uint samples_per_pixel = u_SamplesPerPixel;
uint g_depth = u_Depth;
uint g_Seed = 0;

struct Material
{
    vec4 type;
    vec4 albedo;
    float roughness;
};

struct Sphere
{
    vec4 position; // Radius stored in position.w

    Material mat;
};

layout (std140) uniform ObjectData
{
    int sphereCount;
    Sphere Spheres[4];
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
    Material mat;

    int objectIndex;
};

Ray ComputeRay(vec2 uv)
{
    vec4 clip_pos = vec4(uv.xy, 1.0, 1.0);
    vec4 view_pos = u_InverseProjection * clip_pos;

    // Ray direction in world space
    vec3 d = vec3(u_InverseView * vec4(view_pos.xy, -1.0, 0.0));
    d = normalize(d);

    Ray r;

    r.origin = u_RayOrigin;
    r.direction = d;

    return r;
}

uint PCGHash()
{
    uint state = g_Seed * 747796405 + 2891336453;
    uint word = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
    return (word >> 22) ^ word;
}

float Rand01()
{
    return float(PCGHash()) / 4294967295.0;
}

vec3 RandomSampleUnitSphere()
{
    float theta = Rand01() * 2 * PI;
    float z = Rand01() * 2.0 - 1.0;
    float r = sqrt(1.0 - z * z);
    float x = r * cos(theta);
    float y = r * sin(theta);
    vec3 result = vec3(x, y, z);
    return result;
}

vec3 CosineHemisphereSampling()
{
    float cosine_t = sqrt((1.0 - Rand01()));
    float sine_t = sqrt((1.0 - cosine_t * cosine_t));
    float phi = Rand01() * 2 * PI;
    return vec3(cos(phi) * sine_t, sin(phi) * sine_t, cosine_t);
}

HitRecord ClosestHit(Ray ray, float hitDistance, int objectIndex)
{
    HitRecord rec;
    rec.hitDistance = hitDistance;
    rec.objectIndex = objectIndex;
    
    Sphere closestSphere = objectData.Spheres[objectIndex];

    // Calculate outside normal of sphere and intersection point in world space
    vec3 centre = vec3(closestSphere.position.xyz);
    rec.worldPosition = ray.origin + ray.direction * hitDistance;
    rec.worldNormal = normalize(rec.worldPosition - centre);
    rec.mat = closestSphere.mat; 

    return rec;
}

HitRecord Miss()
{
    HitRecord rec;
    rec.hitDistance = -1;
    return rec;
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

vec3 PerPixel(Ray ray)
{
    vec3 colour = vec3(1.0);
    vec3 attenuation = vec3(0.0);

    for (int i = 0; i < g_depth; i++)
    {
        // Keep track of ray intersection point, direction etc
        HitRecord rec = TraceRay(ray);

        // If ray misses, object takes on colour of the sky
        if (rec.hitDistance < 0)
        {
            vec3 unit_direction = normalize(ray.direction);
            float t = 0.5*(unit_direction.y + 1.0);
            vec3 skyColour = (1.0-t)*vec3(1.0) + t*vec3(0.5, 0.7, 1.0);
            colour *= skyColour;
            break;
        }

        // Closest object to the camera
        Sphere sphere = objectData.Spheres[rec.objectIndex];

        if (rec.mat.type.x == 0)
        {
            // Lambertian Scattering
            ray.origin = rec.worldPosition + rec.worldNormal * 0.001;
            vec3 scattered_dir = rec.worldPosition + rec.worldNormal + RandomSampleUnitSphere();
            ray.direction = normalize(scattered_dir - rec.worldPosition);
            attenuation = sphere.mat.albedo.xyz;
        }
        else if (rec.mat.type.x == 1)
        {
            // Metal Scattering
            vec3 reflected = reflect(normalize(ray.direction), rec.worldNormal);
            ray.direction = reflected + rec.mat.roughness * RandomSampleUnitSphere();
            ray.origin = rec.worldPosition + rec.worldNormal * 0.001;
            (dot(ray.direction, rec.worldNormal) > 0) ? attenuation = sphere.mat.albedo.xyz : attenuation = vec3(0.0);
        }
        
        colour *= attenuation;
        
        // return vec3(rec.worldNormal * 0.5 + 0.5);
    }
    return vec3(colour); 
}

void main()
{
    // Pixel coord in NDC [-1, 1]
    vec2 uv = gl_FragCoord.xy / u_Resolution.xy;
    uv = (uv * 2.0) - 1.0;

    g_Seed = uint(gl_FragCoord.x * 1973 + gl_FragCoord.y * 9277 + u_SampleIterations * 2699) | uint(1);
    
    vec3 pixel_colour = vec3(0.0);

    for (int s = 0; s < samples_per_pixel; s++)
    {
        vec2 jitter = vec2(gl_FragCoord.x + Rand01(), gl_FragCoord.y + Rand01()) / u_Resolution;
        jitter = jitter * 2.0 - 1.0;
        Ray r = ComputeRay(jitter);
        pixel_colour += PerPixel(r);
    }
    pixel_colour /= samples_per_pixel;
    pixel_colour = sqrt(pixel_colour);

    // Progressive rendering:
    // To calculate the cumulative average we must first get the current pixel's data by sampling the accumulation texture 
    // (which holds data of all samples for each pixel which is then averaged out) with the current uv coordinates.
    // Now we scale up the data by the number of samples to this pixel.
    vec4 accumulatedScaledUp = texture(u_AccumulationTexture, (uv + 1.0) / 2.0) * u_SampleIterations;
    // Then we can add the new sample, calculated from the current frame, to the previous samples.
    vec4 newAccumulationContribution = accumulatedScaledUp + vec4(pixel_colour, 1.0);
    // Once we have the new total sum of all samples we can divide (average) by the new number of samples, resulting in 
    // the new average.
    vec4 accumulatedScaledDown = newAccumulationContribution / (u_SampleIterations + 1);
    colour = accumulatedScaledDown;
}
