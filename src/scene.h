#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <string>

#include "primitives.h"
#include "materials.h"
#include "camera.h"
#include "hdri.h"

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
    Scene()
        : maxRayDepth(16)
        , samplesPerPixel(1)
        , day(true)
        , LightIdx(0)
        , PrimitiveIdx(0)
        , SceneIdx(0)
        , envMap(nullptr)
        , envMapIdx(0)
        , envMapHasChanged(false)
        , envMapRotation(0.0f)
        , sunColour(glm::vec3(.992156862745098, .8862745098039216, .6862745098039216))
        , sunElevation(45.0f)
        , sunAzimuth(0.0f) 
    {
        Eye = std::make_unique<Camera>();
        Eye->Reset();
    };
    ~Scene() {};

    SceneBlock Data;
    int maxRayDepth;
    int samplesPerPixel;
    bool day;
    int LightIdx;
    int PrimitiveIdx;
    int SceneIdx;

    std::unique_ptr<HDRI> envMap;
    int envMapIdx;
    bool envMapHasChanged;
    float envMapRotation;

    glm::vec3 sunColour;
    float sunElevation;
    float sunAzimuth;

    std::unique_ptr<Camera> Eye;
    std::vector<Light> lights;
    std::vector<Primitive> primitives;

    void AddDefaultSphere();
    void AddDefaultCube();
    void AddSphere(glm::vec3 position, float radius, Material mat);
    void AddCube(glm::vec3 position, glm::vec3 dimensions, glm::vec3 rotation, Material mat);
    void AddLight(size_t id, glm::vec3 le);
    void AddEnvMap(std::string filepath);

    Material CreateGlassMat(glm::vec3 absorption, float roughness);
    Material CreateDiffuseMat(glm::vec3 albedo, float roughness);
    Material CreateMirrorMat(glm::vec3 albedo, float roughness);
    Material CreateDielectricMat(glm::vec3 albedo, float roughness, float specular);
    Material CreateLightMat(glm::vec3 le, float intensity);

    void SelectScene();
    void WhiteRoomColouredLights();
    void CornellBox();
    void RTIW();
    void NewScene();

    void Init();
    void EmptyScene();
};
