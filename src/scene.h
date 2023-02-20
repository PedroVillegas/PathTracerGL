#pragma once

#include <ctime>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

struct alignas(16) Material
{
    glm::ivec4 type = { 0, 0, 0, 0 };
    glm::vec4 albedo { 1.0f };
    float roughness = 1.0f;
    float ior = 1.0f;
};
struct alignas(16) Sphere
{
    glm::vec4 position { 0.0f, 0.0f, 0.0f, 1.0f };

    Material mat;
};

struct Scene
{
    int maxRayDepth = 16;
    int samplesPerPixel = 1;
    std::vector<Sphere> spheres;

    void Randomize();
};
