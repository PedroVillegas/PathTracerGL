#version 410 core

#define FLT_MAX     3.402823466e+38
#define FLT_MIN     -3.402823466e+38
#define PI          3.14159265358979323
#define EPSILON     0.0001

out vec4 FragColour;

uniform int u_SampleIterations;
uniform int u_SamplesPerPixel;
uniform int u_Depth;
uniform int u_Day;
uniform int u_SelectedObjIdx;
uniform vec3 u_LightDir;
uniform float u_FocalLength;
uniform float u_Aperture;
uniform vec2 u_Resolution;
uniform vec3 u_RayOrigin;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;
uniform mat4 u_ViewProjection;
uniform sampler2D u_AccumulationTexture;

uint samples_per_pixel = u_SamplesPerPixel;
uint g_depth = u_Depth;
uint g_Seed = 0;

struct Material
{
    vec3 albedo;                    // Base Colour
    float ior;                      // Index of Refraction - how refractive it is

    // Emissive properties
    vec3 emissive;                  // How emissive it is
    float emissiveStrength;         // How strong it emits

    // Dielectric properties
    vec3 absorption;                // Absorption for beer's law
    float refractionChance;         // How transparent it is

    // Metallic properties
    vec3 specularTint;              // Colour of reflections
    float specularChance;           // How reflective it is

    float roughness;                // How rough the object is
};

struct AABB
{
    vec3 minCorner;
    vec3 maxCorner;
};

struct Sphere
{
    vec3 position;
    float radius;

    Material mat;
};

layout (std140) uniform ObjectData
{
    int sphereCount;
    Sphere Spheres[256];
} objectData;

struct Ray
{
    vec3 origin;
    vec3 direction;
};

struct Payload
{
    float t; // distance from origin to intersection point along direction 
    vec3 position;
    vec3 normal;
    bool fromInside;
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
    g_Seed += 2743589;
    return float(PCGHash()) / 4294967295.0;
}

vec2 UniformSampleUnitCircle()
{
    float theta = Rand_01() * 2.0 * PI;
    float r = sqrt(Rand_01());
    return vec2(cos(theta), sin(theta)) * r;
}

vec3 UniformSampleUnitSphere(float u1, float u2)
{
    vec3 dir;
    float theta = u1 * 2 * PI;
    float z = u2 * 2.0 - 1.0;
    float r = sqrt(1.0 - z * z);
    float x = r * cos(theta);
    float y = r * sin(theta);
    dir = vec3(x, y, z);

    return dir;
}

vec3 CosineSampleHemisphere(vec3 normal)
{
    vec3 dir;
    vec3 bitangent = normalize(cross(normal, vec3(0.0, 1.0, 1.0)));
	vec3 tangent = cross(bitangent, normal);
	float r = sqrt(Rand_01());
    float phi = 2.0 * PI * Rand_01();
	vec3 x = r * cos(phi) * bitangent; 
	vec3 y = r * sin(phi) * tangent;
	vec3 z = sqrt(1.0 - r*r) * normal;
    dir = x + y + z;
    
    return normalize(dir);
}

float FresnelSchlick(float cosine_t, float n1, float n2)
{
    float r0 = (n1 - n2) / (n1 + n2);
    r0 = r0 * r0;
    return r0 + (1.0-r0) * pow((1.0-cosine_t), 5.0);
}

vec3 FresnelSchlick(float cosine_t, vec3 f0)
{
    return f0 + (1.0-f0) * pow((1.0-cosine_t), 5.0);
}

float FresnelReflectAmount(vec3 V, vec3 N, float n1, float n2)
{
    
    // n1: IOR of medium the ray originated in
    // n2: IOR of medium the ray is entering
    float cosine_t = dot(-V, N);

    if (n1 > n2)
    {
        // Ray originated in a denser medium and is entering a lighter one
        // e.g when exiting a glass sphere
        float eta = n1 / n2;
        float sine_t = sqrt(1.0 - cosine_t * cosine_t);

        if (eta * sine_t > 1.0)
        {
            // No solution to Snell's law => Ray MUST reflect (total internal reflection)
            return 1.0;
        }
    }
    return FresnelSchlick(cosine_t, n1, n2);
}

float D_GGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NdotH2 = NdotH * NdotH;
    float b = (NdotH2 * (alpha2 - 1.0) + 1.0);
    return alpha2 / (PI * b * b);
}

float G1_GGX_Schlick(float NdotV, float roughness)
{
    float alpha = roughness * roughness;
    float k = alpha * 0.5;
    return max(NdotV, 0.001) / (NdotV * (1.0 - k) + k);
}

float G_Smith(float NdotV, float NdotL, float roughness)
{
    return G1_GGX_Schlick(NdotL, roughness) * G1_GGX_Schlick(NdotV, roughness);
}
/*
vec3 CookTorranceBRDF(Payload hitRec, vec3 lightDir, inout vec3 viewDir)
{
    vec3 albedo = hitRec.mat.albedo;
    float roughness = hitRec.mat.roughness;
    float metallic = hitRec.mat.metallic;
    float specular = hitRec.mat.specular;
    vec3 L = lightDir;
    vec3 V = viewDir;
    vec3 N = hitRec.normal;
    // Halfway vector - unit vector halfway between the view and the light direction
    vec3 H = normalize(V + L);
    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    float NdotL = clamp(dot(N, L), 0.0, 1.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float VdotH = clamp(dot(V, H), 0.0, 1.0);

    vec3 f0 = vec3(0.16 * (specular * specular));
    // Lerp between f0 and albedo weighted by the metallic param
    // For a metallic val of 0.0, use dielectric f0; for 1.0, use albedo value
    f0 = mix(f0, albedo, metallic);

    // Computing specular component
    vec3 F = FresnelSchlick(VdotH, f0);
    float D = D_GGX(NdotH, roughness);
    float G = G_Smith(NdotV, NdotL, roughness);

    // Use max to prevent division by zero
    vec3 spec = (F * D * G) / (4.0 * max(NdotV, 0.001) * max(NdotL, 0.001));

    // Computing diffuse component
    // Only the transmitted fraction should contribute to the diffuse component; the transmitted part is 1.0 - F
    // To simulate the fact that metallic surfaces don't diffuse light at all we weight by 1-metallic 
    albedo *= (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diff = albedo / PI;

    return diff + spec;
}
*/
float BSDF(inout Ray ray, Payload hitRec, out bool isRefractive, out float specularFactor)
{
    isRefractive = false;

    float ior = hitRec.mat.ior;
    vec3 N = hitRec.normal;
    vec3 V = ray.direction;

    float diffuseChance;
    float specularChance = hitRec.mat.specularChance;
    float refractionChance = hitRec.mat.refractionChance;   
    float roughness = hitRec.mat.roughness;

    // Account for Fresnel for specularChance and adjust other chances accordingly
    // Specular takes priority
    // chanceMultiplier makes sure we keep diffuse / refraction ratio the same
    if (specularChance > 0.0)
    {
        // n1: ior of the medium the ray start in
        // n2: ior of the medium the ray enters
        float n1 = hitRec.fromInside ? ior : 1.0;
        float n2 = hitRec.fromInside ? 1.0 : ior;

        // x*(1-a) + y*(a)
        float fresnelTerm = FresnelSchlick(dot(-V, N), n1, n2);
        specularChance = mix(specularChance, 1.0, fresnelTerm);

        float chanceMultiplier = (1.0 - specularChance) / (1.0 - hitRec.mat.specularChance);
        refractionChance *= chanceMultiplier;

        diffuseChance = 1.0 - (specularChance + refractionChance);
        refractionChance = 1.0 - (specularChance + diffuseChance);
    }

    // Determine whether to be specular, diffuse or refraction ray
    // and calculate the scattered ray direction accordingly
    specularFactor = 0.0;
    float rayProbability = 1.0;
    float raySelectRoll = Rand_01();
    vec3 diffuseDir = CosineSampleHemisphere(N);
    if (specularChance > 0.0 && raySelectRoll < specularChance)
    {   
        // Reflection ray
        specularFactor = 1.0;
        rayProbability = specularChance;

        // Rough Specular (Glossy) lerps from smooth specular to rough diffuse by the roughness squared
        // Squaring the roughness is done to make the roughness feel more linear perceptually
        vec3 specularDir = reflect(V, N);
        specularDir = normalize(mix(specularDir, diffuseDir, roughness * roughness));
        ray.direction = specularDir;
    }
    else if (refractionChance > 0.0 && raySelectRoll < (specularChance + refractionChance))
    {
        // Refraction ray
        isRefractive = true;
        rayProbability = refractionChance;

        float eta = hitRec.fromInside ? ior : (1.0 / ior);
        vec3 refractionDir = refract(V, N, eta);
        refractionDir = normalize(mix(refractionDir, -diffuseDir, roughness * roughness));
        ray.direction = refractionDir;
    }
    else
    {
        // Diffuse ray
        ray.direction = diffuseDir;
        rayProbability = 1.0 - (specularChance + refractionChance);
    }

    ray.origin = isRefractive ? hitRec.position - N * EPSILON : hitRec.position + N * EPSILON;

    return max(rayProbability, EPSILON);
}

Ray ComputeWorldSpaceRay(vec2 uv)
{
    // Local Space => World Space => View Space => Clip Space => NDC
    vec4 clip_pos = vec4(uv, -1.0, 0.0);
    vec4 view_pos = u_InverseProjection * clip_pos;

    // Ray direction in world space
    vec3 d = vec3(u_InverseView * vec4(view_pos.xy, -1.0, 0.0));
    d = normalize(d);

    Ray r;

    r.origin = u_RayOrigin;
    r.direction = d;

    return r;
}

bool RaySphereIntersect(Ray ray, Sphere sphere, out float t1, out float t2)
{
    t1 = FLT_MAX;
    t2 = FLT_MAX;

    float radius = sphere.radius;
    vec3 OC = ray.origin - sphere.position;
    vec3 V = ray.direction;

    // Evaluate intersection points between ray and sphere
    // by solving quadratic equation
    float b = dot(OC, V);
    float c = dot(OC, OC) - radius * radius;

    // Discriminant hit test (< 0 means no real solution)
    float discriminant = b * b - c;
    if (discriminant < 0.0)
        return false;
    
    discriminant = sqrt(discriminant);
    t1 = -b - discriminant;
    t2 = -b + discriminant;

    return  t1 <= t2;
}

bool RayAABBIntersect(Ray ray, AABB aabb, out float t1, out float t2)
{
    t1 = FLT_MIN;
    t2 = FLT_MAX;

    vec3 inv_D =  1.0 / ray.direction;
    vec3 p1 = aabb.minCorner;
    vec3 p0 = aabb.maxCorner;

    vec3 tLower = (p0 - ray.origin) * inv_D;
    vec3 tUpper = (p1 - ray.origin) * inv_D;

    vec3 tMins = min(tLower, tUpper);
    vec3 tMaxes = max(tLower, tUpper);

    t1 = max(t1, max(tMins.x, max(tMins.y, tMins.z)));
    t2 = min(t2, min(tMaxes.x, min(tMaxes.y, tMaxes.z)));

    return t1 <= t2;
}

AABB SphereBoundingBox(Sphere sphere)
{
    vec3 centre = sphere.position;
    float radius = sphere.radius;

    return AABB(centre - vec3(radius), centre + vec3(radius));
}

Payload ClosestHit(Ray ray, float t, int objectIndex)
{
    Sphere closestSphere = objectData.Spheres[objectIndex];
    float radius = closestSphere.radius;
    Payload HitRec;
    
    HitRec.t = t;
    HitRec.objectIndex = objectIndex;
    
    HitRec.position = ray.origin + ray.direction * t;
    HitRec.mat = closestSphere.mat; 

    vec3 centre = vec3(closestSphere.position);
    vec3 pos = HitRec.position;
    vec3 outward_normal = (pos - centre) / radius;

    // Positive dot product means the vectors point in the same direction
    HitRec.fromInside = dot(ray.direction, outward_normal) > 0.0;
    HitRec.normal = HitRec.fromInside ? -outward_normal : outward_normal;

    return HitRec;
}

Payload Miss()
{
    Payload HitRec;
    HitRec.t = -1;
    return HitRec;
}

Payload TraceRay(Ray ray)
{
    int closestSphereIdx = -1;
    float t = FLT_MAX;
    float t1;
    float t2;

    for (int i = 0; i < objectData.sphereCount; i++)
    {        
        Sphere sphere = objectData.Spheres[i];
        if (RaySphereIntersect(ray, sphere, t1, t2) && t2 > 0.0 && t1 < t)
        {
            t = t1 < 0 ? t2 : t1;
            closestSphereIdx = i;
        }
    }

    if (closestSphereIdx < 0)
        return Miss();

    return ClosestHit(ray, t, closestSphereIdx);
}

vec3 PerPixel(Ray ray)
{
    // Radiance: the radiant flux emitted, reflected, transmitted or received by a given surface
    // Throughput: the amount of radiance passing through the ray
    vec3 radiance = vec3(0.0);
    vec3 throughput = vec3(1.0); 

    for (int i = 0; i < g_depth; i++)
    {
        // Keep track of ray intersection point, direction etc
        Payload HitRec = TraceRay(ray);

        // If ray misses, object takes on radiance of the sky
        if (HitRec.t < 0)
        {
            vec3 V = normalize(ray.direction);
            
            float t = 0.5*(V.y + 1.0);
            vec3 atmosphere;

            u_Day == 1 ? atmosphere = mix(vec3(1.0), vec3(0.50, 0.70, 1.00), t) :   // Day sky
                         atmosphere = mix(vec3(0.0), vec3(0.05, 0.10, 0.30), t);    // Night sky

            radiance += atmosphere * throughput;
            break;
        }

        // Consider emissive objects
        if (HitRec.mat.emissive.x > 0.0 || HitRec.mat.emissive.y > 0.0 || HitRec.mat.emissive.z > 0.0)
        {
            radiance += HitRec.mat.emissive * throughput;
            break;
        }

        // DEBUG: confirm object normals work properly
        // return vec3(HitRec.normal * 0.5 + 0.5);
        if (HitRec.fromInside)
        {
            // Apply beer's law
            throughput *= exp(-HitRec.mat.absorption * HitRec.t);
        }

        // https://blog.demofox.org/2020/06/14/casual-shadertoy-path-tracing-3-fresnel-rough-refraction-absorption-orbit-camera/
        float specularFactor;
        bool isRefractive;
        float rayProbability = BSDF(ray, HitRec, isRefractive, specularFactor);
        // return vec3(rayProbability);

        // Refraction doesn't alter the colour
        if (!isRefractive)
            throughput *= mix(HitRec.mat.albedo, HitRec.mat.specularTint, specularFactor);

        // Since we are choosing to follow only one path (diffuse, specular, refractive)
        // we must account for not choosing the others
        // throughput *= 1.0 / rayProbability;

        // Russian Roulette
        // As throughput gets smaller, the probability (p) of terminating the path early increases
        // given a minimum depth
        // Surviving paths have their value boosted to compensate for fewer samples being in the average
        float p = max(throughput.x, max(throughput.y, throughput.z));
        if (i > 3)
        {
            if (Rand_01() > p)
                break;

            // add energy 'lost' from random path terminations
            throughput *= 1.0 / p;
        }
    }
    return vec3(radiance); 
}

void main()
{
    // Pixel coord in NDC [-1, 1]
    vec2 uv = gl_FragCoord.xy / u_Resolution.xy;
    uv = (uv * 2.0) - 1.0;

    g_Seed = GenerateSeed();
    
    // Irradiance: the radiant flux received by some surface per unit area
    vec3 irradiance = vec3(0.0);

    for (int s = 0; s < samples_per_pixel; s++)
    {
        float r1 = Rand_01();
        float r2 = Rand_01();

        vec2 sub_pixel_jitter = vec2(gl_FragCoord.x + r1, gl_FragCoord.y + r2) / u_Resolution;
        sub_pixel_jitter = sub_pixel_jitter * 2.0 - 1.0;

        Ray r = ComputeWorldSpaceRay(sub_pixel_jitter);
        
        // Compute Lens Defocus Blur
        // First find where the ray hits the focal plane (focal point)
        vec3 focal_point = r.origin + r.direction * u_FocalLength;
        // Pick a random spot on the aperture
        vec2 offset = u_Aperture * 0.5 * UniformSampleUnitCircle();

        // Shoot ray from that random spot towards the focal point
        r.origin += vec3(offset, 0.0);
        r.direction = normalize(focal_point - r.origin);

        irradiance += PerPixel(r);
    }
    irradiance /= samples_per_pixel;
    irradiance = pow(irradiance, vec3(1.0/2.2));

    // Progressive rendering:
    // To calculate the cumulative average we must first get the current pixel's data by sampling the accumulation texture 
    // (which holds data of all samples for each pixel which is then averaged out) with the current uv coordinates.
    // Now we scale up the data by the number of samples to this pixel.
    vec4 accumulatedScaledUp = texture(u_AccumulationTexture, (uv + 1.0) / 2.0) * u_SampleIterations;
    // Then we can add the new sample, calculated from the current frame, to the previous samples.
    vec4 newAccumulationContribution = accumulatedScaledUp + vec4(irradiance, 1.0);
    // Once we have the new total sum of all samples we can divide (average) by the new number of samples, resulting in 
    // the new average.
    vec4 accumulatedScaledDown = newAccumulationContribution / (u_SampleIterations + 1);

    FragColour = accumulatedScaledDown;
}
