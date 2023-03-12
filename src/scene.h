#pragma once

#include <ctime>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

struct alignas(16) Material
{
    // Note diffuse chance = 1.0f so specularChance + refractionChance = 1.0f
    glm::vec3 albedo { 1.0f };
    float ior = 1.0f;
    
    glm::vec3 emissive { 0.0f };
    float emissiveStrength = 0.0f;

    glm::vec3 absorption { 0.0f };
    float refractionChance = 0.0f;
    float refractionRoughness = 0.0f;

    float specularChance = 0.0f;
    float specularRoughness = 0.0f;
};
struct alignas(16) Sphere
{
    glm::vec3 position { 0.0f, 0.0f, 0.0f };
    float radius = 1.0f;

    Material mat;
};

struct Scene
{
    int maxRayDepth = 16;
    int samplesPerPixel = 1;
    char day = 1;
    glm::vec3 lightDirection { 0.0f, 0.0f, 0.0f };
    std::vector<Sphere> spheres;

    void CustomScene();
    void RTIW();
    void RandomizeBRDF();
};
