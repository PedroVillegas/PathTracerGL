#version 410 core

#define FLT_MAX 3.402823466e+38
#define PI 3.14159

#define LAMBERTIAN 0
#define METAL 1
#define DIELECTRIC 2

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
    ivec4 type; // Type of material stored in type.x
    vec4 albedo;
    float roughness;
    float ior;
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

struct Hit_record
{
    float hit_distance;
    vec3 position;
    vec3 normal;
    bool front_face;
    Material mat;

    int objectIndex;
};

// -----------------------------------
// FOR PSEUDO RANDOM NUMBER GENERATION
// -----------------------------------

uint GenerateSeed()
{
    return uint(gl_FragCoord.x * 1973 + gl_FragCoord.y * 9277 + u_SampleIterations * 2699) | uint(1);
}

uint PCGHash()
{
    uint state = g_Seed * 747796405 + 2891336453;
    uint word = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
    return (word >> 22) ^ word;
}

float Rand_01()
{
    return float(PCGHash()) / 4294967295.0;
}

vec3 RandomSampleUnitSphere()
{
    float theta = Rand_01() * 2 * PI;
    float z = Rand_01() * 2.0 - 1.0;
    float r = sqrt(1.0 - z * z);
    float x = r * cos(theta);
    float y = r * sin(theta);
    vec3 result = vec3(x, y, z);
    return result;
}

vec3 CosineHemisphereSampling()
{
    float cosine_t = sqrt((1.0 - Rand_01()));
    float sine_t = sqrt((1.0 - cosine_t * cosine_t));
    float phi = Rand_01() * 2 * PI;
    return vec3(cos(phi) * sine_t, sin(phi) * sine_t, cosine_t);
}

float SchlicksApproximation(float cosine_t, float refraction_ratio)
{
    float r0 = (1-refraction_ratio) / (1+refraction_ratio);
    r0 = r0 * r0;
    return r0 + (1-r0) * pow((1-cosine_t), 5);
}

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

Hit_record ClosestHit(Ray ray, float hit_distance, int objectIndex)
{
    Hit_record Hit_rec;
    Hit_rec.hit_distance = hit_distance;
    Hit_rec.objectIndex = objectIndex;
    
    Sphere closestSphere = objectData.Spheres[objectIndex];

    // Calculate outside normal of sphere and intersection point in world space
    vec3 centre = vec3(closestSphere.position.xyz);
    Hit_rec.position = ray.origin + ray.direction * hit_distance;
    Hit_rec.mat = closestSphere.mat; 
    vec3 outward_normal = normalize(Hit_rec.position - centre);
    Hit_rec.front_face = dot(ray.direction, outward_normal) < 0;
    Hit_rec.normal = Hit_rec.front_face ? outward_normal : -outward_normal;

    return Hit_rec;
}

Hit_record Miss()
{
    Hit_record Hit_rec;
    Hit_rec.hit_distance = -1;
    return Hit_rec;
}

Hit_record TraceRay(Ray ray)
{
    int closestSphereIndex = -1;
    float hit_distance = FLT_MAX;

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
        if (closestRoot > 0.0 && closestRoot < hit_distance)
        {
            hit_distance = closestRoot;
            closestSphereIndex = i;
        }
    }

    if (closestSphereIndex < 0)
        return Miss();

    return ClosestHit(ray, hit_distance, closestSphereIndex);
}

vec3 PerPixel(Ray ray)
{
    vec3 colour = vec3(1.0);
    vec3 attenuation = vec3(0.0); // colour absorbed by objects

    for (int i = 0; i < g_depth; i++)
    {
        // Keep track of ray intersection point, direction etc
        Hit_record Hit_rec = TraceRay(ray);

        // If ray misses, object takes on colour of the sky
        if (Hit_rec.hit_distance < 0)
        {
            vec3 unit_direction = normalize(ray.direction);
            float t = 0.5*(unit_direction.y + 1.0);
            vec3 sky_clr = (1.0-t)*vec3(1.0) + t*vec3(0.5, 0.7, 1.0);
            colour *= sky_clr;
            break;
        }

        // Closest object to the camera
        Sphere sphere = objectData.Spheres[Hit_rec.objectIndex];

        if (Hit_rec.mat.type.x == LAMBERTIAN)
        {
            // Lambertian Scattering
            ray.origin = Hit_rec.position + Hit_rec.normal * 0.001;
            vec3 scattered_dir = Hit_rec.position + Hit_rec.normal + RandomSampleUnitSphere();
            ray.direction = normalize(scattered_dir - Hit_rec.position);
            attenuation = sphere.mat.albedo.xyz;
        }
        else if (Hit_rec.mat.type.x == METAL)
        {
            // Metal Scattering
            vec3 reflected = reflect(normalize(ray.direction), Hit_rec.normal);
            ray.direction = reflected + Hit_rec.mat.roughness * RandomSampleUnitSphere();
            ray.origin = Hit_rec.position + Hit_rec.normal * 0.001;
            (dot(ray.direction, Hit_rec.normal) > 0) ? attenuation = sphere.mat.albedo.xyz : attenuation = vec3(0.0);
        }
        
        else if (Hit_rec.mat.type.x == DIELECTRIC)
        {
            // Glass Scattering
            attenuation = vec3(1.0); // Attenuation always 1 since glass doesn't absorb anything
            vec3 n = Hit_rec.normal;
            vec3 d = normalize(ray.direction);
            float ior = Hit_rec.mat.ior;

            // When the incident ray intersects object from the outside, the ray goes from air (ior = 1.0) -> dielectric thus the
            // ratio is the inverse of the objects ior, otherwise, the ratio is ior/1.0
            float refraction_ratio = Hit_rec.front_face ? (1.0 / ior) : ior;

            float cosine_t = min(dot(-d, n), 1.0);
            float sine_t = sqrt(1.0 - cosine_t * cosine_t);

            // When the incident ray is inside the glass and outside is air, the refraction ratio will be greater than 1.0, thus
            // there is no solution to Snell's law { sin(t_1) = (ior_1 / ior_2) * sine(t_2) } when the RHS > 1.0 because sine(t_1) 
            // can't be greater than 1.0. When no solution is available, the glass cannot refract and therefore must reflect
            bool cannot_refract = refraction_ratio * sine_t > 1.0;
            vec3 direction;
            vec3 position;

            // When looking at glass from a steep angle (near perpendicular) it becomes a mirror. This is caused by the
            // Fresnel ('Fre-nel') effect which suggests that reflections are weak when the incident ray is at angles closer
            // to parallel to the normal and strong at angles closer to perpendicular to the normal. The coefficient of this
            // effect can be approximated via 'Schlicks Approximation'
            if (cannot_refract || SchlicksApproximation(cosine_t, refraction_ratio) > Rand_01())
            {
                direction = reflect(d, n);
                position = Hit_rec.position + n * 0.001;
            }
            else
            {
                direction = refract(d, n, refraction_ratio);
                position = Hit_rec.position - n * 0.001;
            }

            ray.origin = position;
            ray.direction = direction;
        }
        
        colour *= attenuation;
        
        // return vec3(Hit_rec.normal * 0.5 + 0.5);
    }
    return vec3(colour); 
}

void main()
{
    // Pixel coord in NDC [-1, 1]
    vec2 uv = gl_FragCoord.xy / u_Resolution.xy;
    uv = (uv * 2.0) - 1.0;

    g_Seed = GenerateSeed();
    
    vec3 pixel_colour = vec3(0.0);

    for (int s = 0; s < samples_per_pixel; s++)
    {
        vec2 jitter = vec2(gl_FragCoord.x + Rand_01(), gl_FragCoord.y + Rand_01()) / u_Resolution;
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
