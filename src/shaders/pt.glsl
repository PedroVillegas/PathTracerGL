#version 460 core

#include <common/structs.glsl>
#include <common/data.glsl>
#include <common/utils.glsl>
#include <common/ray_gen.glsl>
#include <common/miss.glsl>
#include <common/closest_hit.glsl>
#include <common/any_hit.glsl>
#include <common/bsdf.glsl>

#define LIGHT_SPHERE 0
#define LIGHT_AREA 1
#define SUN_ENABLED
#define SUN_COLOUR vec3(.992156862745098, .8862745098039216, .6862745098039216)
#define RUSSIAN_ROULETTE_MIN_BOUNCES 5

out vec4 FragColour;

vec3 EstimateDirect(Light light, Payload payload, Ray ray)
{
    vec3 directIlluminance = vec3(0.0);
    Primitive primitive = Prims.Primitives[light.id];
    if (!all(greaterThan(primitive.mat.emissive, vec3(0.0)))) return directIlluminance;

    // Sample a point on the primitive
    float pdf;
    vec3 sampledPos = SamplePointOnPrimitive(primitive, pdf, payload.position);

    // Test visibility
    vec3 wi = normalize(sampledPos - payload.position);
    float cos_term = dot(wi, payload.normal);
    if (cos_term <= 0) return directIlluminance;

    // Cast shadow ray from surface to light
    Ray SR = Ray(payload.position + payload.normal * 0.001, wi);
    Payload shadowInfo;
    if (AnyHit(SR, shadowInfo, distance(SR.origin, sampledPos)) 
        && shadowInfo.primID == light.id && distance(shadowInfo.position, sampledPos) < 0.1)
    {
        // Convert area pdf to solid angle pdf
        float r = shadowInfo.t;
        float cos_term = abs(cos_term);
        pdf = (r*r) / cos_term * pdf;
        if (pdf < 0.01) return directIlluminance;
        directIlluminance += (EvalBSDF(payload, ray, wi) * cos_term * primitive.mat.emissive * primitive.mat.intensity) / pdf;
    }

    return directIlluminance;
}

// https://computergraphics.stackexchange.com/questions/5152/progressive-path-tracing-with-explicit-light-sampling
vec3 SampleLights(Payload hitrec, Ray ray, bool lastBounceSpecular)
{
    vec3 directIlluminance = vec3(0.0);

    for (int i = 0; i < Prims.n_Lights; i++)
    {
        Light light = Prims.Lights[i];

        // If the light and the surface interaction are the same we skip
        // so we don't double dip
        if (light.id == hitrec.primID) continue;

        // Accumulate direct lighting
        directIlluminance += EstimateDirect(light, hitrec, ray);
    }
    return directIlluminance;
}

vec3 SampleSun(Payload shadingPoint, Ray ray, bool lastBounceSpecular)
{
    vec3 directIlluminance = vec3(0.0);

#ifdef SUN_ENABLED
    if (Scene.Day == 1)
    {
        vec3 dir = normalize(Scene.SunDirection);
        // Omega_i is the incoming light
        vec3 wi = normalize(GetConeSample(dir, 1e-5));

        // Test visibility
        float cos_term = dot(wi, shadingPoint.normal);
        if (cos_term <= 0.0) return directIlluminance;

        // Cast shadow ray from surface to light
        Ray SR = Ray(shadingPoint.position + shadingPoint.normal * 0.001, wi);
        Payload shadowInfo;
        if (!lastBounceSpecular && !AnyHit(SR, shadowInfo, INF))
        {
            // Only sample if bsdf is non-specular (refl or refr)
            directIlluminance += EvalBSDF(shadingPoint, ray, wi) * Scene.SunColour * abs(cos_term);
        }
    }
#endif
    return directIlluminance;
}

vec4 PathTrace(Ray ray)
{
    // Radiance: the radiant flux emitted, reflected, transmitted or received by a given surface
    // Throughput: the amount of radiance passing through the ray
    vec3 radiance = vec3(0.0);
    vec3 throughput = vec3(1.0); 
    bool lastBounceSpecular = false;
    float nodeVisits = 0.0;
    // float bvhDepthReached = 0.0;
    
    for (int bounce = 0; bounce < Scene.Depth; bounce++)
    {
        // Keep track of ray intersection point, direction etc
        Payload HitRec = ClosestHit(ray, INF, nodeVisits);

        // If ray misses, object takes on radiance of the sky
        if (HitRec.t == INF)
        {
            radiance += Miss(ray.direction) * throughput;
            break;
        }

        if (u_DebugBVHVisualisation == 1)
        {
            radiance = vec3(0.0);
            // radiance = HitRec.mat.albedo;
            break;
        }

        // return vec4(1.0, 0.0, 1.0, 1.0);
        // return vec4(vec3(HitRec.t/100.0), 1.0);
        
        // Consider emissive materials
        // if (any(greaterThan(HitRec.mat.emissive, vec3(0.0))))
        if ((bounce == 0 || lastBounceSpecular == true) && any(greaterThan(HitRec.mat.emissive, vec3(0.0))))
        {
            radiance += HitRec.mat.emissive * HitRec.mat.intensity * throughput;
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
        vec3 direct = SampleLights(HitRec, ray, lastBounceSpecular) + SampleSun(HitRec, ray, lastBounceSpecular);
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
    // Debug: Visualize BVH Bounding Boxes
    if (u_DebugBVHVisualisation == 1)
        radiance.rgb += (nodeVisits / 255.) * 8.;

    return vec4(radiance, 1.0); 
}

void main()
{
    // Pixel coord in NDC [-1, 1]
    vec2 uv = gl_FragCoord.xy / u_Resolution.xy;
    uv = (uv * 2.0) - 1.0;

    g_Seed = GenerateSeed();
    
    // Irradiance: the radiant flux received by some surface per unit area
    vec4 irradiance = vec4(0.0);

    int spp = u_SamplesPerPixel;
    for (int s = 0; s < spp; s++)
    {
        float r_1 = Randf01();
        float r_2 = Randf01();

        vec2 subPixelOffset = vec2(r_1, r_2);
        vec2 ndc = (gl_FragCoord.xy + subPixelOffset) / u_Resolution * 2.0 - 1.0;

        Ray r = RayGen(ndc);
    
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
    vec4 newAccumulationContribution = accumulatedScaledUp + irradiance;
    // Once we have the new total sum of all samples we can divide (average) by the new number of samples, resulting in 
    // the new average.
    vec4 accumulatedScaledDown = newAccumulationContribution / (u_SampleIterations + 1);

    FragColour = accumulatedScaledDown;
    // FragColour = vec4(Randf01(), Randf01(), Randf01(), 1.0);
}
