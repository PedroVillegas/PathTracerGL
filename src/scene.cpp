#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <string>

#include "primitives.h"
#include "materials.h"
#include "scene.h"

float Randf01()
{
    return (float)(std::rand()) / (float)(RAND_MAX);
}

glm::vec3 RandVec3()
{
    return glm::vec3(Randf01(), Randf01(), Randf01());
}

void Scene::EmptyScene()
{
    primitives.clear();
    lights.clear();
}

void Scene::Init()
{
    int n_Primitives = primitives.size();
    for (int n = 0; n < n_Primitives; n++)
    {
        primitives[n].id = n;
        if (primitives[n].mat.emissive != glm::vec3(0.0f))
        {
            AddLight(primitives[n].id, primitives[n].mat.emissive);
        }
    }

    // for (auto prim : primitives)
    // {
    //     std::cout << "id  : " << prim.id << std::endl;
    //     std::cout << "type: " << prim.type << std::endl;
    //     std::cout << "posx: " << prim.position.x << std::endl;
    //     std::cout << "posy: " << prim.position.y << std::endl;
    //     std::cout << "posz: " << prim.position.z << std::endl;
    //     std::cout << std::endl;
    // }
}

void Scene::AddDefaultSphere()
{
    Primitive sphere;
    sphere.id = primitives.size();
    sphere.type = PRIM_SPHERE;
    sphere.position = glm::vec3(0.0f, 0.0f, 0.0f);
    sphere.radius = 1.0f;
    sphere.mat.albedo = glm::vec3(1.0f);
    primitives.push_back(sphere);
}

void Scene::AddDefaultCube()
{
    Primitive cube;
    cube.id = primitives.size();
    cube.type = PRIM_AABB;
    cube.position = glm::vec3(0.0f, 0.0f, 0.0f);
    cube.dimensions = glm::vec3(2.0f);
    cube.mat.albedo = glm::vec3(1.0f);
    primitives.push_back(cube);
}

void Scene::AddSphere(glm::vec3 position, float radius, Material mat)
{
    Primitive sphere;
    sphere.type = PRIM_SPHERE;
    sphere.position = position;
    sphere.radius = radius;
    sphere.mat = mat;
    primitives.push_back(sphere);
}

void Scene::AddCube(glm::vec3 position, glm::vec3 dimensions, Material mat)
{
    Primitive cube;
    cube.type = PRIM_AABB;
    cube.position = position;
    cube.dimensions = dimensions;
    cube.mat = mat;
    primitives.push_back(cube);
}

void Scene::AddLight(int id, glm::vec3 le)
{
    Light light;
    light.id = id;
    light.le = le;
    lights.push_back(light);
}

Material Scene::CreateGlassMat(glm::vec3 absorption, float ior, float roughness)
{
    Material out = Material();
    out.refractionChance = 1.0f;
    out.specularChance = 0.02f;
    out.absorption = absorption;
    out.ior = ior;
    out.roughness = roughness;
    
    return out;
}

Material Scene::CreateDiffuseMat(glm::vec3 albedo, float roughness)
{
    Material out = Material();
    out.albedo = albedo;
    out.roughness = roughness;
    
    return out;
}

Material Scene::CreateMirrorMat(glm::vec3 albedo, float roughness)
{
    Material out = Material();
    out.specularChance = 1.0f;
    out.albedo = albedo;
    out.roughness = roughness;
    
    return out;
}

Material Scene::CreateDielectricMat(glm::vec3 albedo, float roughness, float specular)
{
    Material out = Material();
    out.albedo = albedo;
    out.roughness = roughness;
    out.specularChance = specular;
    
    return out;
}

void Scene::RTIW()
{
    using namespace glm;

    float EPSILON = 1e-3f;
    float lightHeight = 8.0f;
    float lightWidth = 24.0f;
    float lightDepth = 12.0f;
    float panelThickness = 0.1f;

    Material panel = Material(vec3(0.5f), 0.0f, vec3(0.0f), 0.0f, vec3(0.0f), 0.0f, 1.0f, 1.0f);
    // AddCube(vec3(0.0f, lightHeight + 0.5f * panelThickness + EPSILON, 0.0f), 
    //         vec3(1.25f * lightWidth, panelThickness, 2.0f * lightDepth), panel);
    
    Material floorMat = Material(vec3(1.0f, 0.95f, 0.8f), 0.0f, vec3(0.0f), 0.0f, vec3(0.0f), 0.0f, 1.0f, 1.0f);
    AddCube(vec3(0.0f), vec3(100'000.0f, 0.01f, 100'000.0f), floorMat);

    Material GLASS = CreateGlassMat(vec3(0.3f, 0.5f, 0.4f), 1.55f, 0.15f);
    AddSphere(vec3(-6.0f, 3.0f, 0.0f), 3.0f, GLASS);

    Material BlueDiffuse = CreateDiffuseMat(vec3(0.1f, 0.2f, 0.5f), 1.0f);
    AddSphere(vec3(0.0f, 3.0f, 0.0f), 3.0f, BlueDiffuse);

    // Gold vec3(0.8f, 0.6f, 0.2f)
    Material MetalBall = CreateMirrorMat(vec3(0.5f), 0.15f);
    AddSphere(vec3(6.0f, 3.0f, 0.0f), 3.0f, MetalBall);

    Init();
}

void Scene::CornellBox()
{
    using namespace glm;

    lightDirection = vec3(-0.55f, 0.2f, 1.0f);
    vec3 WHITE_COL = vec3(.7295, .7355, .729)*0.7f;
    vec3 RED_COL   = vec3(.611, .0555, .062)*0.7f;
    vec3 GREEN_COL = vec3(.117, .4125, .115)*0.7f;
    vec3 BLUE_COL   = vec3(0.08f, 0.16f, 0.29f);

    Material redDiffuse   = CreateDiffuseMat(RED_COL, 1.0f);
    Material greenDiffuse = CreateDiffuseMat(GREEN_COL, 1.0f);
    Material blueDiffuse  = CreateDiffuseMat(BLUE_COL, 1.0f);
    Material whiteDiffuse = CreateDiffuseMat(WHITE_COL, 1.0f);

    // Left Wall
    AddCube(vec3(-5.0f, 5.0f, 0.0f), vec3(0.01f, 10.0f, 10.0f), redDiffuse);

    // Right Wall
    AddCube(vec3( 5.0f, 5.0f, 0.0f), vec3(0.01f, 10.0f, 10.0f), blueDiffuse);

    // Back Wall
    AddCube(vec3(0.0f, 5.0f, -5.0f), vec3(10.0f, 10.0f, 0.01f), whiteDiffuse);

    // Ceiling
    AddCube(vec3(0.0f, 10.0f, 0.0f), vec3(10.0f, 0.01f, 10.0f), whiteDiffuse);

    // Ground
    AddCube(vec3(0.0f), vec3(1e5f, 0.01f, 1e5f), whiteDiffuse);

    Material GLASS = CreateGlassMat(vec3(0.3f, 0.2f, 0.1f), 1.55f, 0.1f);
    AddSphere(vec3(-2.0f, 2.5f, -2.0f), 2.0f, GLASS);

    Material MIRROR = CreateMirrorMat(vec3(0.7f), 0.1f);
    // AddSphere(vec3(2.0f, 7.0f, 2.0f), 2.0f, MIRROR);
    // AddCube(vec3(0.0f, 2.0f, 0.0f), vec3(4.0f), MIRROR);

    Material light = Material(vec3(1.0f), 0.0f, vec3(1.0f), 1.0f, vec3(0.0f), 0.0f, 0.0f, 0.0f);
    // AddSphere(vec3(0.0f, 8.0f, 0.0f), 1.0f, light);
    // AddCube(vec3(0.0f, 10.0f - 0.01f, 0.0f), vec3(2.5f, 0.01f, 2.5f), light);

    Init();
}
