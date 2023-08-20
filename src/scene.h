#pragma once

#include <vector>
#include "primitives.h"

struct alignas(16) Light
{
    glm::vec3 position { 0.0f };
    float radius = 1.0f;
    glm::vec3 dimensions { 1.0f };
    int type = 0;
    glm::vec3 emissive { 0.0f };
    int geomID;
};

struct Scene
{
    int maxRayDepth = 16;
    int samplesPerPixel = 1;
    char day = 0;
    int SphereIdx = 0;
    int AABBIdx = 0;
    int SceneIdx = 0;
    glm::vec3 lightDirection { 0.0f, 0.0f, 0.0f };
    std::vector<Sphere> spheres;
    std::vector<GPUAABB> aabbs;
    std::vector<Light> lights;

    void GridShowcase();
    void CustomScene();
    void ModifiedCornellBox();
    void CornellBox();
    void RTIW();
    void RandomizeBRDF();
    void emptyScene();
};
