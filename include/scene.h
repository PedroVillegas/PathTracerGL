#pragma once

#include <glm/glm.hpp>

#include <vector>

struct alignas(16) Sphere
{
    glm::vec4 Position {0.0f};
    float Radius = 1.0f;
    glm::vec4 Albedo = glm::vec4(1.0f);
};

struct Scene
{
    std::vector<Sphere> Spheres;
    glm::vec3 lightDirection {-1.0f};
};
