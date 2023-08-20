#version 410 core

// #include <structs.glsl> in data.glsl
#include <data.glsl>
#include <utils.glsl>

#include <miss.glsl>
#include <traceRay.glsl>

out vec4 FragColour;

uint g_depth = u_Depth;

float FresnelSchlick(float cosine_t, float n_1, float n_2)
{
    float r_0 = (n_1 - n_2) / (n_1 + n_2);
    r_0 = r_0 * r_0;
    return r_0 + (1.0 - r_0) * pow((1.0 - cosine_t), 5.0);
}

float FresnelSchlick(float cosine_t)
{
    return pow((1.0-cosine_t), 5.0);
}

float FresnelReflectAmount(float n1, float n2, vec3 N, vec3 V, float f_0, float f_90)
{
        // Schlick aproximation
        float r0 = (n1-n2) / (n1+n2);
        r0 *= r0;
        float cosX = -dot(N, V);
        if (n1 > n2)
        {
            float n = n1/n2;
            float sinT2 = n*n*(1.0-cosX*cosX);
            // Total internal reflection
            if (sinT2 > 1.0)
                return f_90;
            cosX = sqrt(1.0-sinT2);
        }
        float x = 1.0-cosX;
        float ret = r0+(1.0-r0)*x*x*x*x*x;
 
        // adjust reflect multiplier for object reflectivity
        return mix(f_0, f_90, ret);
}

vec3 EvalBSDF(Payload hitrec, inout Ray ray, out float pdf)
{
    vec3 V = ray.direction;
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
        // n_1: ior of the medium the ray start in
        // n_2: ior of the medium the ray enters
        float n_1 = hitrec.fromInside ? ior : 1.0;
        float n_2 = hitrec.fromInside ? 1.0 : ior;

        // x*(1-a) + y*(a)
        // float F = FresnelSchlick(abs(dot(V, N)) + EPSILON, n_1, n_2);
        float F = FresnelReflectAmount(n_1, n_2, N, V, specularChance, 1.0);
        specularChance = mix(specularChance, 1.0, F);
        // return vec3(F);

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

    if (specularChance > 0.0 && raySelectRoll < specularChance)
    {   
        // Reflection ray
        specularFactor = 1.0;
        rayProbability = specularChance;

        // Rough Specular (Glossy) lerps from smooth specular to rough diffuse by the roughness squared
        // Squaring the roughness is done to make the roughness feel more linear perceptually
        vec3 specularDir = reflect(V, N);
        specularDir = normalize(mix(specularDir, diffuseDir, roughness * roughness));
        scattered = specularDir;
    }
    else if (refractionChance > 0.0 && raySelectRoll < (specularChance + refractionChance))
    {
        // Refraction ray
        refractiveFactor = 1.0;
        rayProbability = refractionChance;

        float eta = hitrec.fromInside ? ior : (1.0 / ior);
        vec3 refractionDir = refract(V, N, eta);
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

    pdf = max(rayProbability, EPSILON);
    // return vec3(pdf);
    pdf = 1.0;

    return mix(mix(albedo, vec3(1.0), specularFactor), vec3(1.0), refractiveFactor);
}

vec3 RayGen(Ray ray)
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

        // DEBUG Normals
        // return vec3(HitRec.normal * 0.5 + 0.5);

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
            if (Randf01() > rrp) 
                break;
            else 
                throughput *= 1.0 / rrp; // add energy 'lost' from random path terminations
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

    uint spp = u_SamplesPerPixel;
    for (int s = 0; s < spp; s++)
    {
        float r_1 = Randf01();
        float r_2 = Randf01();

        vec2 subPixelOffset = vec2(r_1, r_2) - 0.5;
        vec2 ndc = (gl_FragCoord.xy + subPixelOffset) / u_Resolution * 2.0 - 1.0;

        Ray r = TransformToWorldSpace(ndc);
    
        // Compute Bokeh Blur (Depth of Field)
        // First find where the ray hits the focal plane (focal point)
        vec3 focal_point = r.origin + r.direction * u_FocalLength;
        // Pick a random spot on the lens (aperture)
        vec2 offset = u_Aperture * 0.5 * UniformSampleUnitCircle(r_1, r_2);

        // Shoot ray from that random spot towards the focal point
        r.origin += vec3(offset, 0.0);
        r.direction = normalize(focal_point - r.origin);

        irradiance += RayGen(r);
    }

    irradiance /= spp;

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
