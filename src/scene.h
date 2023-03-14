#pragma once

#include <ctime>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <string>

struct alignas(16) Material // 65 bytes (aligned as vec4s)
{
    // Note diffuse chance = 1.0f so specularChance + refractionChance = 1.0f
    glm::vec3 albedo { 1.0f };
    float ior = 1.0f;
    
    glm::vec3 emissive { 0.0f };
    float emissiveStrength = 0.0f;

    glm::vec3 absorption { 0.0f };
    float refractionChance = 0.0f;

    glm::vec3 specularTint { 1.0f };
    float specularChance = 0.0f;
    float roughness = 0.0f;
};

struct alignas(16) GPUSphere // 81 bytes naturally aligned
{
    glm::vec3 position { 0.0f, 0.0f, 0.0f };
    float radius = 1.0f;

    Material mat; // 65 bytes
};

struct Sphere
{
    std::string label; 
    GPUSphere sphere;
};

struct Scene
{
    int maxRayDepth = 16;
    int samplesPerPixel = 1;
    char day = 1;
    glm::vec3 lightDirection { 0.0f, 0.0f, 0.0f };
    std::vector<Sphere> spheres;

    void CircleScene();
    void CustomScene();
    void RTIW();
    void RandomizeBRDF();
};
