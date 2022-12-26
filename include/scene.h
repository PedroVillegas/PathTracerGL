#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Material
{
    glm::vec4 Albedo = glm::vec4(1.0f);
    //float Roughness = 1.0f;
    //float Metallic = 1.0f;
};
struct alignas(16) Sphere
{
    glm::vec4 Position { 0.0f, 0.0f, 0.0f, 1.0f };

    Material Mat;
};

struct Scene
{
    std::vector<Sphere> Spheres;
    glm::vec3 lightDirection = glm::vec3(-1.0f);
};
