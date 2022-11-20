#pragma once

#include <glm/glm.hpp>

#include <vector>

struct alignas(16) Sphere
{
    glm::vec4 Position {0.0f, 0.0f, 0.0f, 1.0f};
    float Radius = 0.5f;
    glm::vec4 Albedo {1.0f, 1.0f, 1.0f, 1.0f};
};

struct Scene
{
    std::vector<Sphere> Spheres;
    glm::vec3 lightDirection {-1.0f, -1.0f, -1.0f};
};
