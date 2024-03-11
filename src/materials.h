#pragma once

#include <glm/glm.hpp>

struct alignas(16) Material
{
    Material()
        : albedo(glm::vec3(1.0f))
        , roughness(1.0f)
        , emissive(glm::vec3(0.0f))
        , intensity(0.0f)
        , absorption(glm::vec3(0.0f))
        , ior(1.0f)
        , metallic(0.0f)
        , transmission(0.0f)
    {};

    Material(
        glm::vec3 albedo,
        glm::vec3 emissive, float intensity,
        float roughness, float metallic,
        float transmission, float ior, glm::vec3 absorption
    )
        : albedo(albedo)
        , emissive(emissive)
        , intensity(intensity)
        , metallic(metallic)
        , roughness(roughness)
        , transmission(transmission)
        , ior(ior)
        , absorption(absorption)
    {};

    glm::vec3 albedo;
    float roughness;
    glm::vec3 emissive;
    float intensity;
    glm::vec3 absorption;
    float ior;
    float metallic;
    float transmission;

};