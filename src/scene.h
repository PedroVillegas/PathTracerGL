#pragma once

#include <vector>
#include "primitives.h"

struct alignas(16) Light
{
    int type;
    int PrimitiveOffset;
    int pad0;
    int pad1;
};

struct SceneBlock
{
    glm::vec3 SunDirection;
    int pad;
    int Depth;
    int SelectedPrimIdx;
    int Day;
};

const uint32_t MAX_SPHERES = 100;
const uint32_t MAX_AABBS = 100;
const uint32_t MAX_LIGHTS = 100;

struct Scene
{
    SceneBlock Data;
    int maxRayDepth = 16;
    int samplesPerPixel = 1;
    char day = 0;
    int SphereIdx = 0;
    int AABBIdx = 0;
    int SceneIdx = 0;
    glm::vec3 lightDirection { -1.0f, 1.0f, 1.0f };
    std::vector<GPUSphere> spheres;
    std::vector<GPUAABB> aabbs;
    std::vector<Light> lights;
    std::vector<Primitive> primitives;

    void AddDefaultSphere();
    void AddDefaultCube();

    void CornellBox();
    void RTIW();
    void TestPrims();
    void emptyScene();
};
