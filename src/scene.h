#pragma once

#include <ctime>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <string>

struct alignas(16) Material
{
    // Note diffuse chance = 1.0f so specularChance + refractionChance = 1.0f
    glm::vec3 albedo { 1.0f };
    float specularChance = 0.0f;
    glm::vec3 emissive { 0.0f };
    float emissiveStrength = 0.0f;
    glm::vec3 absorption { 0.0f };
    float refractionChance = 0.0f;
    float ior = 1.0f;
    float metallic = 0.0f;
    float roughness = 1.0f;

    Material() = default;

    Material(glm::vec3 albedo, float specularChance,
             glm::vec3 emissive, float emissiveStrength,
             glm::vec3 absorption, float refractionChance,
             float ior, float roughness)
    : albedo(albedo)
    , specularChance(glm::clamp(specularChance, 0.0f, 1.0f))
    , emissive(emissive)
    , emissiveStrength(emissiveStrength)
    , absorption(absorption)
    , refractionChance(glm::clamp(refractionChance, 0.0f, 1.0f - specularChance))
    , ior(glm::max(1.0f, ior))
    , roughness(roughness)
    {}
};

struct alignas(16) Light
{
    glm::vec3 position { 0.0f };
    float radius = 1.0f;

    glm::vec3 emissive { 0.0f };
    int geomID;
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

    Light GetLight()
    {
        Light light;
        light.position = sphere.position;
        light.radius = sphere.radius;
        light.emissive = sphere.mat.emissive;
        light.geomID = sphere.geomID;
        return light;
    }
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
