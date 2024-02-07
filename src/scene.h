#pragma once

#include <vector>
#include <memory>
#include "primitives.h"
#include "camera.h"

struct alignas(16) Light
{
    int id;
    alignas(16) glm::vec3 le;
};

struct SceneBlock
{
    glm::vec3 SunDirection;
    float pad;
    glm::vec3 SunColour;
    float pad1;
    int Depth;
    int SelectedPrimIdx;
    int Day;
};

const uint32_t MAX_PRIMITIVES = 100;
const uint32_t MAX_LIGHTS = 100;

class Scene
{
public:
    Scene();
    ~Scene();

    SceneBlock Data;
    int maxRayDepth = 16;
    int samplesPerPixel = 1;
    char day = 1;
    int LightIdx = 0;
    int PrimitiveIdx = 0;
    int SceneIdx = 0;

    glm::vec3 sunColour = glm::vec3(.992156862745098, .8862745098039216, .6862745098039216);
    float sunElevation = 10.0f;
    float sunAzimuth = 330.0f;
    std::vector<Light> lights;
    std::vector<Primitive> primitives;

    std::unique_ptr<Camera> Eye;

    void AddDefaultSphere();
    void AddDefaultCube();
    void AddSphere(glm::vec3 position, float radius, Material mat);
    void AddCube(glm::vec3 position, glm::vec3 dimensions, Material mat);
    void AddLight(size_t id, glm::vec3 le);

    Material CreateGlassMat(glm::vec3 absorption, float ior, float roughness);
    Material CreateDiffuseMat(glm::vec3 albedo, float roughness);
    Material CreateMirrorMat(glm::vec3 albedo, float roughness);
    Material CreateDielectricMat(glm::vec3 albedo, float roughness, float specular);
    Material CreateLightMat(glm::vec3 le, float intensity);

    void SelectScene();
    void WhiteRoomColouredLights();
    void CornellBox();
    void RTIW();
    void Init();
    void EmptyScene();
};
