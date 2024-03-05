#pragma once

#include <glm/glm.hpp>

struct alignas(16) Material
{
    glm::vec3 albedo { 1.0f };
    float roughness = 1.0f;
    glm::vec3 emissive { 0.0f };
    float intensity = 0.0f;
    float metallic = 0.0f;

    Material() = default;

    Material(glm::vec3 albedo,
             glm::vec3 emissive, float intensity,
             float metallic, float roughness)
    : albedo(albedo)
    , emissive(emissive)
    , intensity(intensity)
    , metallic(metallic)
    , roughness(roughness)
    {}
};