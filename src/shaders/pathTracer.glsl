#version 410 core

#define FLT_MAX     3.402823466e+38
#define FLT_MIN     -3.402823466e+38
#define PI          3.14159265358979323
#define EPSILON     1e-3

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
uniform vec3 u_ObjectCounts;
uniform mat4 u_InverseProjection;
uniform mat4 u_InverseView;
uniform mat4 u_ViewProjection;
uniform sampler2D u_AccumulationTexture;

uint samples_per_pixel = u_SamplesPerPixel;
uint g_depth = u_Depth;
uint g_Seed = 0;
// vec3 g_ObjectCounts = u_ObjectCounts;

struct Material
{
    vec3 albedo;                    // Base Colour
    float specularChance;           // How reflective it is
    vec3 emissive;                  // How emissive it is
    float emissiveStrength;         // How strong it emits
    vec3 absorption;                // Absorption for beer's law
    float refractionChance;         // How transparent it is
    float ior;                      // Index of Refraction - how refractive it is
    float metallic;                 // A material is either metallic or it's not
    float roughness;                // How rough the object is
};

struct AABB
{
    vec3 position;
    float pad0;
    vec3 dimensions;
    float pad1;

    Material mat;
};

struct Sphere
{
    vec3 position;
    float radius;
    vec3 padding;
    int geomID;

    Material mat;
};

struct Light
{
    vec3 position;
    float radius;
    vec3 dimensions;
    int type;
    vec3 emissive;
    int geomID;
};

layout (std140) uniform ObjectData
{
    int n_Spheres;
    Sphere Spheres[50];
    int n_AABBs;
    AABB aabbs[50];
    int n_Lights;
    Light Lights[50];
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
    int geomID;
};

//----------------------------------RNG----------------------------------

uint GenerateSeed()
{
    return uint(gl_FragCoord.x * 1973 + gl_FragCoord.y * 9277 + u_SampleIterations * 2699) | uint(1);
}

uint PCGHash()
{
    uint state = g_Seed;
    g_Seed *= 747796405 + 2891336453;
    uint word = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
    return (word >> 22) ^ word;
}

float Randf01()
{
    return float(PCGHash()) / 4294967295.0;
}

//----------------------------------RNG----------------------------------

//--------------------------Sphere/Disc Sampling-------------------------

vec2 UniformSampleUnitCircle(float r1, float r2)
{
    float theta = r1 * 2.0 * PI;
    float r = sqrt(r2);
    return vec2(cos(theta), sin(theta)) * r;
}

vec3 UniformSampleUnitSphere(float u1, float u2)
{
    vec3 dir;
    float theta = u1 * 2.0 * PI;
    float z = u2 * 2.0 - 1.0;
    float r = sqrt(1.0 - z * z);
    float x = r * cos(theta);
    float y = r * sin(theta);
    dir = vec3(x, y, z);

    return normalize(dir);
}

vec3 CosineSampleHemisphere(float r1, float r2, vec3 N)
{
    vec3 dir;
    vec3 B = normalize(cross(N, vec3(0.0, 1.0, 1.0)));
	vec3 T = cross(B, N);
	float r = sqrt(r1);
    float phi = 2.0 * PI * r2;
	vec3 x = r * cos(phi) * B; 
	vec3 y = r * sin(phi) * T;
	vec3 z = sqrt(1.0 - r*r) * N;
    dir = x + y + z;
    
    return normalize(dir);
}

vec3 NoTangentCosineHemisphere(float u1, float u2, vec3 N) 
{
    return normalize(N + UniformSampleUnitSphere(u1, u2));
}

//--------------------------Sphere/Disc Sampling-------------------------

float FresnelSchlick(float cosine_t, float n1, float n2)
{
    float r0 = (n1 - n2) / (n1 + n2);
    r0 = r0 * r0;
    return r0 + (1.0-r0) * pow((1.0-cosine_t), 5.0);
}

float FresnelSchlick(float cosine_t)
{
    return pow((1.0-cosine_t), 5.0);
}

vec3 EvalBSDF(Payload hitrec, inout Ray ray, out float pdf)
{
    vec3 V = -ray.direction;
    vec3 N = hitrec.normal;

    vec3 scattered;
    vec3 albedo = hitrec.mat.albedo;
    float specularChance = hitrec.mat.specularChance;
    float refractionChance = hitrec.mat.refractionChance;  
    float roughness = hitrec.mat.roughness;
    float metallic = hitrec.mat.metallic;
    float ior = hitrec.mat.ior;

    // Account for Fresnel for specularChance and adjust other chances accordingly
    // Specular takes priority
    // chanceMultiplier makes sure we keep diffuse / refraction ratio the same
    if (specularChance > 0.0)
    {
        // n1: ior of the medium the ray start in
        // n2: ior of the medium the ray enters
        float n1 = hitrec.fromInside ? ior : 1.0;
        float n2 = hitrec.fromInside ? 1.0 : ior;

        // x*(1-a) + y*(a)
        float f = FresnelSchlick(abs(dot(V, N)), n1, n2);
        specularChance = mix(specularChance, 1.0, f);
        // return vec3(specularChance);

        // Need to maintain the same probability ratio for refraction and diffuse later.
        refractionChance *= (1.0 - specularChance) / (1.0 - hitrec.mat.specularChance);
    }

    // Determine whether to be specular, diffuse or refraction ray
    // and calculate the scattered ray direction accordingly
    float specularFactor = 0.0;
    float refractiveFactor = 0.0;
    float rayProbability = 1.0;
    float raySelectRoll = Randf01();
    vec3 diffuseDir = NoTangentCosineHemisphere(Randf01(), Randf01(), N);

    if (raySelectRoll < specularChance)
    {   
        // Reflection ray
        specularFactor = 1.0;
        rayProbability = specularChance;

        // Rough Specular (Glossy) lerps from smooth specular to rough diffuse by the roughness squared
        // Squaring the roughness is done to make the roughness feel more linear perceptually
        vec3 specularDir = reflect(-V, N);
        specularDir = normalize(mix(specularDir, diffuseDir, roughness * roughness));
        scattered = specularDir;
    }
    else if (raySelectRoll < (specularChance + refractionChance))
    {
        // Refraction ray
        refractiveFactor = 1.0;
        rayProbability = refractionChance;

        float eta = hitrec.fromInside ? ior : (1.0 / ior);
        vec3 refractionDir = refract(-V, N, eta);
        refractionDir = normalize(mix(refractionDir, -diffuseDir, roughness * roughness));
        scattered = refractionDir;
    }
    else
    {
        // Diffuse ray
        scattered = diffuseDir;
        rayProbability = 1.0 - (specularChance + refractionChance);
    }

    ray.origin = bool(refractiveFactor) ? hitrec.position - N * 0.001 : hitrec.position + N * 0.001;
    ray.direction = scattered;

    // pdf = max(rayProbability, EPSILON);
    pdf = 1.0;

    return mix(mix(albedo, mix(vec3(1.0), albedo, metallic), specularFactor), vec3(1.0), refractiveFactor);
}

int GetLightIndex(int geomID)
{
    if (objectData.n_Lights == 0) return -1;

    for (int i = 0; i < objectData.n_Lights; i++)
    {
        if (geomID == objectData.Lights[i].geomID)
            return i;
    }
}

bool RaySphereIntersect(Ray ray, Sphere sphere, out float t1, out float t2)
{
    t1 = FLT_MAX;
    t2 = FLT_MAX;

    float radius = sphere.radius;
    vec3 OC = ray.origin - sphere.position;
    vec3 V = -ray.direction;

    // Evaluate intersection points between ray and sphere
    // by solving quadratic equation
    float b = dot(OC, -V);
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
    vec3 bmin = aabb.position - aabb.dimensions * 0.5;
    vec3 bmax = aabb.position + aabb.dimensions * 0.5;

    vec3 tLower = (bmin - ray.origin) * inv_D;
    vec3 tUpper = (bmax - ray.origin) * inv_D;

    vec3 tMins = min(tLower, tUpper);
    vec3 tMaxes = max(tLower, tUpper);

    t1 = max(t1, max(tMins.x, max(tMins.y, tMins.z)));
    t2 = min(t2, min(tMaxes.x, min(tMaxes.y, tMaxes.z)));

    return t1 <= t2;
}

vec3 GetAABBNormal(AABB aabb, vec3 surfacePosition)
{
    vec3 bmin = aabb.position - aabb.dimensions * 0.5;
    vec3 bmax = aabb.position + aabb.dimensions * 0.5;

    vec3 halfSize = (bmax - bmin) * 0.5;
    vec3 centerSurface = surfacePosition - (bmax + bmin) * 0.5;
    
    vec3 normal = vec3(0.0);
    normal += vec3(sign(centerSurface.x), 0.0, 0.0) * step(abs(abs(centerSurface.x) - halfSize.x), EPSILON);
    normal += vec3(0.0, sign(centerSurface.y), 0.0) * step(abs(abs(centerSurface.y) - halfSize.y), EPSILON);
    normal += vec3(0.0, 0.0, sign(centerSurface.z)) * step(abs(abs(centerSurface.z) - halfSize.z), EPSILON);
    return normalize(normal);
}

vec3 GetSphereNormal(Sphere sphere, vec3 surfacePosition)
{
    return (surfacePosition - sphere.position) / sphere.radius;
}

// AABB SphereBoundingBox(Sphere sphere)
// {
//     vec3 centre = sphere.position;
//     float radius = sphere.radius;

//     return AABB(centre - vec3(radius), centre + vec3(radius));
// }

Ray ComputeWorldSpaceRay(vec2 uv)
{
    // Local Space => World Space => View Space => Clip Space => NDC
    vec4 ndc = vec4(uv, -1.0, 1.0);
    vec4 clip_pos = u_InverseProjection * ndc;
    clip_pos.zw = vec2(-1.0, 0.0);

    // Ray direction in world space
    vec3 d = normalize(u_InverseView * clip_pos).xyz;

    Ray r;

    r.origin = u_RayOrigin;
    r.direction = d;

    return r;
}

vec3 Miss(vec3 V)
{           
    float t = 0.7*(V.y + 1.0);
    vec3 horizonColour = 1.2*vec3(1.00,0.90,0.83);
    vec3 skyColour = vec3(0.40,0.75,1.00);
    vec3 atmosphere;

    u_Day == 1 
            ? atmosphere = mix(horizonColour, skyColour, t)
            : atmosphere = vec3(0.0);

    return atmosphere;
}

Payload TraceRay(Ray ray)
{
    Payload hitrec;
    hitrec.t = FLT_MAX;
    float t1;
    float t2;

    for (int i = 0; i < objectData.n_Spheres; i++)
    {        
        Sphere sphere = objectData.Spheres[i];
        if (RaySphereIntersect(ray, sphere, t1, t2) && t2 > 0.01 && t1 < hitrec.t)
        {
            hitrec.t = t1 < 0 ? t2 : t1;
            hitrec.position = ray.origin + ray.direction * hitrec.t;
            hitrec.mat = sphere.mat; 
            hitrec.fromInside = hitrec.t == t2;
            vec3 outward_normal = GetSphereNormal(sphere, hitrec.position);
            hitrec.normal = hitrec.fromInside ? -outward_normal : outward_normal;
        }
    }

    for (int i = 0; i < objectData.n_AABBs; i++)
    {        
        AABB aabb = objectData.aabbs[i];
        if (RayAABBIntersect(ray, aabb, t1, t2) && t2 > 0.01 && t1 < hitrec.t)
        {
            hitrec.t = t1 < 0 ? t2 : t1;
            hitrec.position = ray.origin + ray.direction * hitrec.t;
            hitrec.mat = aabb.mat;
            hitrec.normal = GetAABBNormal(aabb, hitrec.position);
            hitrec.fromInside = hitrec.t == t2;
        }
    }

    return hitrec;
}

vec3 DirectLighting()
{
    return vec3(1.0);
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
        if (HitRec.t == FLT_MAX)
        {
            radiance += Miss(normalize(ray.direction)) * throughput;
            break;
        }
        
        // Consider emissive materials
        if (HitRec.mat.emissive.x + HitRec.mat.emissive.y + HitRec.mat.emissive.z != 0.0)
        {
            radiance += HitRec.mat.emissive * HitRec.mat.emissiveStrength * throughput;
            break;
        }

        // DEBUG: confirm object normals work properly
        // return vec3(HitRec.normal * 0.5 + 0.5);

        // Direct Lighting
        // vec3 d = DirectLighting();

        // radiance += d * throughput;

        // https://blog.demofox.org/2020/06/14/casual-shadertoy-path-tracing-3-fresnel-rough-refraction-absorption-orbit-camera/
        if (HitRec.fromInside)
        {
            // Apply beer's law
            throughput *= exp(-HitRec.mat.absorption * HitRec.t);
        }

        float pdf;
        vec3 bsdf = EvalBSDF(HitRec, ray, pdf);
        // return bsdf;
        throughput *= bsdf / pdf;
        // ray.direction = scattered;
        // ray.origin = HitRec.position + ray.direction * 0.001;

        /* Russian Roulette */
        // As throughput gets smaller, the probability (rrp) of terminating the path early increases
        // Surviving paths have their value boosted to compensate for terminated paths
        float rrp = min(0.95, max(throughput.x, max(throughput.y, throughput.z)));
        if (i > 3)
        {
            if (Randf01() > rrp) break;
            else throughput *= 1.0 / rrp; // add energy 'lost' from random path terminations
        }
    }
    return radiance; 
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
        float r1 = Randf01();
        float r2 = Randf01();

        vec2 subPixelOffset = vec2(r1, r2) - 0.5;
        vec2 ndc = (gl_FragCoord.xy + subPixelOffset) / u_Resolution * 2.0 - 1.0;

        Ray r = ComputeWorldSpaceRay(ndc);
    
        // Compute Lens Defocus Blur
        // First find where the ray hits the focal plane (focal point)
        vec3 focal_point = r.origin + r.direction * u_FocalLength;
        // Pick a random spot on the aperture
        vec2 offset = u_Aperture * 0.5 * UniformSampleUnitCircle(r1, r2);

        // Shoot ray from that random spot towards the focal point
        r.origin += vec3(offset, 0.0);
        r.direction = normalize(focal_point - r.origin);

        irradiance += PerPixel(r);
    }

    irradiance /= samples_per_pixel;

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
    // FragColour = vec4(Randf01(), Randf01(), Randf01(), 1.0);
}
