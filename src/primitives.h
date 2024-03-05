#pragma once

#include <string>
#include <glm/glm.hpp>

#include "materials.h"

struct AABB;

enum 
{
    PRIM_SPHERE = 0,
    PRIM_AABB = 1
};

struct alignas(16) Primitive
{
    int id;
    int type;
    alignas(16) glm::vec3 position;
    float radius;
    alignas(16) glm::vec3 dimensions;
    Material mat;    

    void BoundingBox(AABB* out);
};

struct AABB
{
    AABB()
    {  
        bMin = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
        bMax = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);
    }

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
