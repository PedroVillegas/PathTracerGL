#version 460 core

#include <structs.glsl>
#include <data.glsl>
#include <utils.glsl>
#include <bvh.glsl>
#include <miss.glsl>
#include <tracePrims.glsl>
#include <bsdf.glsl>

#define LIGHT_SPHERE 0
#define LIGHT_AREA 1
#define SUN_ENABLED
#define SUN_COLOUR vec3(16.86, 10.76, 8.2)*200.
#define RUSSIAN_ROULETTE_MIN_BOUNCES 3

out vec4 FragColour;

vec3 EstimateDirect(Light light, Payload payload, Ray ray)
{
    vec3 DL = vec3(0.0);
    Primitive primitive = Prims.Primitives[light.id];

    // Sample a point on the primitive
    float pdf;
    vec3 sampledPos = SamplePointOnPrimitive(primitive, pdf, payload.position);

    // Test visibility
    vec3 wi = normalize(sampledPos - payload.position);
    float cosine_t = dot(wi, payload.normal);
    if (cosine_t <= 0) return DL;

    Ray SR = Ray(payload.position + payload.normal * 0.001, wi);
    Payload shadowInfo;
    // if (TracePrimsHit(SR, shadowInfo, INF) && shadowInfo.primID == light.id && distance(shadowInfo.position, sampledPos) < 0.1)
    if (!TracePrimsHit(SR, shadowInfo, distance(SR.origin, sampledPos)))
    {
        DL += EvalBSDF(payload, ray, wi) * cosine_t * light.le * primitive.mat.emissiveStrength / pdf;
    }

    return DL;
}

// https://computergraphics.stackexchange.com/questions/5152/progressive-path-tracing-with-explicit-light-sampling
vec3 SampleLights(Payload hitrec, Ray ray, bool lastBounceSpecular)
{
    vec3 DL = vec3(0.0);

#ifdef SUN_ENABLED
    vec3 dir = normalize(Scene.SunDirection);
    // Omega_i is the incoming light
    vec3 wi = GetConeSample(dir, 1e-5);

    // Test visibility
    float cos_term = dot(wi, hitrec.normal);
    if (cos_term <= 0.0) return DL;

    // Generate shadow ray surface to light
    Ray SR = Ray(hitrec.position + hitrec.normal * 0.001, wi);
    Payload shadowInfo;
    if (!TracePrimsHit(SR, shadowInfo, INF) || shadowInfo.mat.refractionChance == 1.0)
    {
        // Only sample if bsdf is non-specular
        DL += EvalBSDF(hitrec, ray, wi) * vec3(1.0);// * cos_term;
    }
    return DL;
#endif

    for (int i = 0; i < Prims.n_Lights; i++)
    {
        Light light = Prims.Lights[i];

        // If the light and the surface interaction are the same we skip
        // so we don't double dip
        if (light.id == hitrec.primID) continue;

        // Accumulate direct lighting
        DL += EstimateDirect(light, hitrec, ray);
    }
    return DL;
}

vec3 PathTrace(Ray ray)
{
    // Radiance: the radiant flux emitted, reflected, transmitted or received by a given surface
    // Throughput: the amount of radiance passing through the ray
    vec3 radiance = vec3(0.0);
    vec3 throughput = vec3(1.0); 
    bool lastBounceSpecular = false;
    
    for (int bounce = 0; bounce < Scene.Depth; bounce++)
    {
        // Keep track of ray intersection point, direction etc
    #ifdef BVH_ENABLED
        Payload payload;
        if (TraverseBVH(ray, payload))
        {
            return payload.position;
            return vec3(1.0, 0.0, 1.0);
        }
    #endif
        Payload HitRec = TracePrims(ray, INF);
        // return vec3(HitRec.t);

        // If ray misses, object takes on radiance of the sky
        if (HitRec.t == INF)
        {
            radiance += Miss(normalize(ray.direction)) * throughput;
            break;
        }
        
        // Consider emissive materials
        if (bounce == 0 && any(greaterThan(HitRec.mat.emissive, vec3(0.0))))
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

        // Calculate direct lighting
        vec3 direct = SampleLights(HitRec, ray, lastBounceSpecular);
        radiance += throughput * direct;

        // Calculate indirect lighting
        vec3 indirect = EvalIndirect(HitRec, ray, lastBounceSpecular);
        throughput *= indirect;

        /* Russian Roulette */
        // As throughput gets smaller, the probability (rrp) of terminating the path early increases
        // Surviving paths have their value boosted to compensate for terminated paths
        float rrp = min(0.95, max(throughput.x, max(throughput.y, throughput.z)));
        if (bounce > RUSSIAN_ROULETTE_MIN_BOUNCES)
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

    int spp = u_SamplesPerPixel;
    for (int s = 0; s < spp; s++)
    {
        float r_1 = Randf01();
        float r_2 = Randf01();

        vec2 subPixelOffset = vec2(r_1, r_2);
        vec2 ndc = (gl_FragCoord.xy + subPixelOffset) / u_Resolution * 2.0 - 1.0;

        Ray r = TransformToWorldSpace(ndc);
    
        // Compute Bokeh Blur (Depth of Field)
        // First find where the ray hits the focal plane (focal point)
        vec3 focal_point = r.origin + r.direction * Camera.focalLength;
        // Pick a random spot on the lens (aperture)
        vec2 offset = Camera.aperture * 0.5 * SampleUniformUnitCirle(r_1, r_2);

        // Shoot ray from that random spot towards the focal point
        r.origin += vec3(offset, 0.0);
        r.direction = normalize(focal_point - r.origin);

        irradiance += PathTrace(r);
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
