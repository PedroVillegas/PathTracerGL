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
    float radius{ 1.0f };
    alignas(16) glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    alignas(16) glm::mat4 rotation{ 1.0f };
    alignas(16) glm::mat4 inverseRotation{ 1.0f };
    alignas(16) glm::vec3 dimensions{ 1.0f };
    Material mat;
    alignas(16) glm::vec3 euler{ 0.0f };

    void UpdateRotation()
    {
        rotation = RotateX(glm::radians(euler.y)) * RotateY(glm::radians(euler.x)) * RotateZ(glm::radians(euler.z));
        inverseRotation = glm::inverse(rotation);
    }

    glm::mat4 RotateX(float theta)
    {
        // Rotate about the x axis
        glm::mat4 Rx = glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, glm::cos(theta), -glm::sin(theta), 0.0f,
            0.0f, glm::sin(theta), glm::cos(theta), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
        return Rx;
    }

    glm::mat4 RotateY(float theta)
    {
        // Rotate about the y axis
        glm::mat4 Ry = glm::mat4(
            glm::cos(theta), 0.0f, glm::sin(theta), 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -glm::sin(theta), 0.0f, glm::cos(theta), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
        return Ry;
    }

    glm::mat4 RotateZ(float theta)
    {
        // Rotate about the z axis
        glm::mat4 Rz = glm::mat4(
            glm::cos(theta), -glm::sin(theta), 0.0f, 0.0f,
            glm::sin(theta), glm::cos(theta), 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
        return Rz;
    }

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
