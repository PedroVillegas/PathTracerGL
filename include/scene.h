#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Material
{
    glm::vec4 albedo { 1.0f };
    float roughness = 1.0f;
    //float metallic = 1.0f;
};
struct alignas(16) Sphere
{
    glm::vec4 position { 0.0f, 0.0f, 0.0f, 1.0f };

    Material mat;
};

struct Scene
{
    int maxRayDepth = 16;
    std::vector<Sphere> spheres;
    glm::vec3 lightDirection { 0.0f };
};
