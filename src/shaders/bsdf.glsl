// #include <structs.glsl>
// #include <utils.glsl>

vec3 Lambertian(vec3 Diffuse, vec3 N, vec3 L)
{
    return (Diffuse / PI) * dot(N, L);
}

float LambertianPDF(vec3 N, vec3 L)
{
    return dot(N, L) / PI;
}

vec3 EvalBSDF(Payload hitrec, Ray ray, vec3 L)
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

    // specularChance = 0.0;
    // refractionChance = 0.0;

    if (specularChance > 0.0)
    {
        float n_1 = hitrec.fromInside ? ior : 1.0;
        float n_2 = hitrec.fromInside ? 1.0 : ior;
        float F = FresnelReflectAmount(n_1, n_2, N, V, specularChance, 1.0);
        specularChance = mix(specularChance, 1.0, F);
    }

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
        rayProbability = 1.0;

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

    vec3 lambertian = Lambertian(albedo, N, L) / LambertianPDF(N, L);

    return mix(mix(lambertian, vec3(1.0), specularFactor), vec3(1.0), refractiveFactor);
}

vec3 EvalIndirect(Payload hitrec, inout Ray ray)
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

    // refractionChance = 0.0;
    // specularChance = 0.0;

    // Account for Fresnel for specularChance and adjust other chances accordingly
    // Specular takes priority
    if (specularChance > 0.0)
    {
        // n_1: ior of the medium the ray start in
        // n_2: ior of the medium the ray enters
        float n_1 = hitrec.fromInside ? ior : 1.0;
        float n_2 = hitrec.fromInside ? 1.0 : ior;

        float F = FresnelReflectAmount(n_1, n_2, N, V, specularChance, 1.0);
        specularChance = mix(specularChance, 1.0, F);
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
        rayProbability = 1.0;

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

    return mix(mix(albedo, vec3(1.0), specularFactor), vec3(1.0), refractiveFactor);
}