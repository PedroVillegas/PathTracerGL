#version 460 core
#extension GL_EXT_scalar_block_layout: require

float Saturate( float x ) { return clamp( x, 0.0, 1.0 ); }

#define LIGHT_SPHERE 0
#define LIGHT_AREA 1
#define SUN_ENABLED
#define SUN_COLOUR vec3(.992156862745098, .8862745098039216, .6862745098039216)
#define SUN_SUNSET vec3(182, 126, 91) / 255.0
#define SUN_INTENSITY 25.0
#define RUSSIAN_ROULETTE_MIN_BOUNCES 5

#include <common/structs.glsl>
#include <common/data.glsl>
#include <common/utils.glsl>
#include <common/ray_gen.glsl>
#include <common/miss.glsl>
#include <common/intersect.glsl>
#include <common/closest_hit.glsl>
#include <common/any_hit.glsl>
#include <common/bsdf.glsl>
#include <common/pbr.glsl>

out vec4 FragColour;

vec3 InfernoQuintic(float x)
{
	x = Saturate( x );
	vec4 x1 = vec4( 1.0, x, x * x, x * x * x ); // 1 x x2 x3
	vec4 x2 = x1 * x1.w * x; // x4 x5 x6 x7
	return vec3(
		dot( x1.xyzw, vec4( -0.027780558, +1.228188385, +0.278906882, +3.892783760 ) ) + dot( x2.xy, vec2( -8.490712758, +4.069046086 ) ),
		dot( x1.xyzw, vec4( +0.014065206, +0.015360518, +1.605395918, -4.821108251 ) ) + dot( x2.xy, vec2( +8.389314011, -4.193858954 ) ),
		dot( x1.xyzw, vec4( -0.019628385, +3.122510347, -5.893222355, +2.798380308 ) ) + dot( x2.xy, vec2( -3.608884658, +4.324996022 ) ) );
}

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
    if (cos_term == 0) return directIlluminance;

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
        float brdf_pdf;
        directIlluminance += (EvalBRDF(ray, payload, wi, brdf_pdf) * cos_term * primitive.mat.emissive * primitive.mat.intensity) / pdf;
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
        vec3 wi = normalize(GetConeSample(dir, 1e-5));

        // Test visibility
        float cos_term = dot(wi, shadingPoint.normal);
        if (cos_term == 0.0) return directIlluminance;

        // Cast shadow ray from surface to light
        Ray SR = Ray(shadingPoint.position + shadingPoint.normal * 0.001, wi);
        Payload shadowInfo;
        if (!AnyHit(SR, shadowInfo, INF))
        {
            // Only sample if bsdf is non-specular (refl or refr)
            float pdf = 1.0;
            //directIlluminance += EvalBSDF(shadingPoint, ray, wi) * Scene.SunColour * abs(cos_term) * SUN_INTENSITY;
            directIlluminance += EvalBRDF(ray, shadingPoint, wi, pdf) * Scene.SunColour * abs(cos_term) * SUN_INTENSITY / pdf;
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
        /* Russian Roulette */
//        if (bounce >= RUSSIAN_ROULETTE_MIN_BOUNCES)
//        {
//            //float rrp = min(0.95, max(throughput.x, max(throughput.y, throughput.z)));
//            float rrp = min(0.95, Luma(throughput));
//            if (Randf01() > rrp) break;
//            else throughput /= rrp;
//        }

        // Keep track of ray intersection point, direction etc
        Payload HitRec = ClosestHit(ray, INF, nodeVisits);

        if (u_DebugBVHVisualisation == 1)
        {
            radiance = nodeVisits > 0 ? InfernoQuintic(nodeVisits / u_TotalNodes) : vec3(0.0);
            // radiance = HitRec.mat.albedo;
            break;
        }

        // If ray misses, object takes on radiance of the sky
        if (HitRec.t == INF)
        {
            radiance += Miss(ray.direction) * throughput;
            break;
        }

        // return vec4(1.0, 0.0, 1.0, 1.0);
        // return vec4(vec3(HitRec.t/100.0), 1.0);
        
        // Consider emissive materials
        //if (any(greaterThan(HitRec.mat.emissive, vec3(0.0))))
        if ((bounce == 0 || lastBounceSpecular) && any(greaterThan(HitRec.mat.emissive, vec3(0.0))))
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
            //throughput *= exp(-HitRec.mat.absorption * HitRec.t);
        }

        // Calculate direct lighting
        vec3 direct = SampleLights(HitRec, ray, lastBounceSpecular) + SampleSun(HitRec, ray, lastBounceSpecular);
        radiance += throughput * direct;

        // Calculate indirect lighting
        vec3 indirect;
        float BRDF_pdf = 1.0;
        indirect = EvalIndirectBRDF(ray, HitRec, BRDF_pdf);
        throughput *= indirect / BRDF_pdf;
    }
    // Debug: Visualize BVH Bounding Boxes
//    float normalizationFactor = u_TotalNodes;
//    if (u_DebugBVHVisualisation == 1)
//        radiance.rgb = InfernoQuintic(nodeVisits / normalizationFactor);

    return vec4(radiance, 1.0); 
}

void main()
{
    // Pixel coord in NDC [-1, 1]
    uv = gl_FragCoord.xy / u_Resolution.xy;
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
//    FragColour = vec4(Randf01(), Randf01(), Randf01(), 1.0);
}
