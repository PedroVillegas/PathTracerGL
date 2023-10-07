#pragma once

#include <glm/glm.hpp>

struct alignas(16) Material
{
    // Note diffuse chance = 1.0f so specularChance + refractionChance = 1.0f
    glm::vec3 albedo { 1.0f };
    float specularChance = 0.0f;
    glm::vec3 emissive { 0.0f };
    float intensity = 0.0f;
    glm::vec3 absorption { 0.0f };
    float refractionChance = 0.0f;
    float ior = 1.0f;
    float metallic = 0.0f;
    float roughness = 1.0f;
    float pad;

    Material() = default;

    Material(glm::vec3 albedo, float specularChance,
             glm::vec3 emissive, float intensity,
             glm::vec3 absorption, float refractionChance,
             float ior, float roughness)
    : albedo(albedo)
    , specularChance(glm::clamp(specularChance, 0.0f, 1.0f))
    , emissive(emissive)
    , intensity(intensity)
    , absorption(absorption)
    , refractionChance(glm::clamp(refractionChance, 0.0f, 1.0f - specularChance))
    , ior(glm::max(1.0f, ior))
    , roughness(roughness)
    {}
};