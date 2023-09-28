#version 410 core

#include <structs.glsl>
#include <data.glsl>
#include <utils.glsl>
#include <bvh.glsl>
#include <miss.glsl>
#include <traceRay.glsl>
#include <tracePrims.glsl>
#include <bsdf.glsl>
#include <anyHit.glsl>

#define LIGHT_SPHERE 0
#define LIGHT_AREA 1

out vec4 FragColour;

vec3 EstimateDirect(Sphere light, Payload hitrec, Ray ray)
{
    vec3 DL = vec3(0.0);
    vec3 dir = normalize(light.position - hitrec.position);
    vec3 Li = vec3(1.0);// light.mat.emissive * light.mat.emissiveStrength;
    if (any(notEqual(Scene.SunDirection, vec3(0.0)))) dir = normalize(Scene.SunDirection);
    vec3 SampledDir = GetConeSample(dir, 1e-5);
    
    // Generate shadow ray surface to light
    Ray SR;
    SR.origin = hitrec.position + hitrec.normal * 0.001;
    SR.direction = SampledDir;
    vec3 SamplePointOnLight = SR.origin + SR.direction * (distance(light.position, hitrec.position) - light.radius);
    float DistanceFromLight = distance(SR.origin, SamplePointOnLight);
    float SunLight = dot(SampledDir, hitrec.normal);
    if (SunLight > 0.0 && !AnyHit(SR, DistanceFromLight))
    {
        // Only sample if bsdf is non-specular
        DL += EvalBSDF(hitrec, ray, SampledDir) * Li;
    }
    return DL;
}

vec3 EstimateDirect(AABB light, Payload hitrec, Ray ray)
{
    vec3 DL = vec3(0.0);
    vec3 Li = light.mat.emissive;// * light.mat.emissiveStrength;
    vec3 SamplePointOnLight = GetAreaLightSample(light);
    vec3 SampledDir = normalize(SamplePointOnLight - hitrec.position);

    // Generate shadow ray surface to light
    Ray SR;
    SR.origin = hitrec.position;
    SR.direction = SampledDir;
    float DistanceFromLight = distance(SR.origin, SamplePointOnLight);
    if (!AnyHit(SR, DistanceFromLight))
    {
        // Only sample if bsdf is non-specular
        DL += EvalBSDF(hitrec, ray, SampledDir) * Li;
    }
    return DL;
}

// https://computergraphics.stackexchange.com/questions/5152/progressive-path-tracing-with-explicit-light-sampling
vec3 SampleLights(Payload hitrec, Ray ray)
{
    vec3 DL = vec3(0.0);
    for (int i = 0; i < Prims.n_Lights; i++)
    {
        Light l = Prims.Lights[i];
        if (l.type == LIGHT_SPHERE)
        {
            Sphere light = Prims.Spheres[l.PrimitiveOffset];
            DL += EstimateDirect(light, hitrec, ray);
        }
        if (l.type == LIGHT_AREA)
        {
            AABB light = Prims.aabbs[l.PrimitiveOffset];
            DL += EstimateDirect(light, hitrec, ray);
        }
    }
    return DL;
}

vec3 RayGen(Ray ray)
{
    // Radiance: the radiant flux emitted, reflected, transmitted or received by a given surface
    // Throughput: the amount of radiance passing through the ray
    vec3 radiance = vec3(0.0);
    vec3 throughput = vec3(1.0); 
    
    for (int bounce = 0; bounce < Scene.Depth; bounce++)
    {
        // Keep track of ray intersection point, direction etc
        // Payload HitRec = TraceRay(ray);
        Payload HitRec = TracePrims(ray);
        // return vec3(HitRec.t);

        // If ray misses, object takes on radiance of the sky
        if (HitRec.t == FLT_MAX)
        {
            radiance += Miss(normalize(ray.direction)) * throughput;
            break;
        }
        
        // Consider emissive materials
        if (any(greaterThan(HitRec.mat.emissive, vec3(0.0))))
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
        // vec3 direct = SampleLights(HitRec, ray);
        // radiance += throughput * direct;
        // return direct;

        // Calculate indirect lighting
        vec3 indirect = EvalIndirect(HitRec, ray);
        throughput *= indirect;

        /* Russian Roulette */
        // As throughput gets smaller, the probability (rrp) of terminating the path early increases
        // Surviving paths have their value boosted to compensate for terminated paths
        float rrp = min(0.95, max(throughput.x, max(throughput.y, throughput.z)));
        if (bounce > 3)
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
        vec3 focal_point = r.origin + r.direction * Camera.focalLength;
        // Pick a random spot on the lens (aperture)
        vec2 offset = Camera.aperture * 0.5 * UniformSampleUnitCircle(r_1, r_2);

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
    // FragColour = vec4(Camera.position,1.0);
    // FragColour = vec4(Randf01(), Randf01(), Randf01(), 1.0);
}
