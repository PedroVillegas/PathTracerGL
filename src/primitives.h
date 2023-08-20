#pragma once

#include <string>
#include <glm/glm.hpp>

#include "materials.h"

struct AABB;

struct alignas(16) GPUSphere
{
    glm::vec3 position { 0.0f };
    float radius = 1.0f;
    glm::vec3 padding { 0.0f };
    int geomID;

    Material mat;
};

struct Sphere
{
    std::string label; 
    GPUSphere sphere;

    void BoundingBox(AABB* out);
};

struct alignas(16) GPUAABB
{
    glm::vec3 position = glm::vec3(0.0f);
    float pad0 = 0.0f;
    glm::vec3 dimensions = glm::vec3(1.0f);
    float pad1 = 0.0f;

    Material mat;

    GPUAABB() = default;
    GPUAABB(glm::vec3 position, glm::vec3 dimensions, Material mat)
    : position(position)
    , dimensions(dimensions)
    , mat(mat) {}
};

struct AABB
{
    glm::vec3 bMin;
    glm::vec3 bMax;

    int LongestAxis() const 
    {
        // Get longest axis
        glm::vec3 d = bMax - bMin;
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }
};

AABB Union(const AABB& b0, const AABB& b1);
AABB Union(const AABB& b, const glm::vec3& p);