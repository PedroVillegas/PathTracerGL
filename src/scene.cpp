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

void Scene::emptyScene()
{
    spheres.clear();
    aabbs.clear();
    lights.clear();
}

void Scene::AddDefaultSphere()
{
    GPUSphere s;
    s.position = glm::vec3(0.0f);
    s.radius = 1.0f;
    s.mat.albedo = glm::vec3(1.0f);
    spheres.push_back(s);
}

void Scene::AddDefaultCube()
{
    GPUAABB cube;
    cube.position = glm::vec3(0.0f);
    cube.dimensions = glm::vec3(1.0f);
    aabbs.push_back(cube);
}

void Scene::RTIW()
{
    int OffsetAABB = 0;

    float floorThickness = 0.5f;
    Material floorMat = Material(glm::vec3(1.0f, 0.95f, 0.8f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    GPUAABB floor = GPUAABB(
        glm::vec3(0.0f, -(1.0f + 0.5f * floorThickness), 0.0f),
        glm::vec3(100'000.0f, floorThickness, 100'000.0f), floorMat);
    aabbs.push_back(floor);
    OffsetAABB++;

    float EPSILON = 1e-3f;
    float lightHeight = 4.0f;
    float lightWidth = 4.0f;
    float lightDepth = 1.0f;
    float panelThickness = 0.1f;

    Material panel = Material(glm::vec3(0.5f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    GPUAABB ceiling = GPUAABB(
        glm::vec3(0.0f, lightHeight + 0.5f * panelThickness + EPSILON, 0.0f),
        glm::vec3(1.25f * lightWidth, panelThickness, 2.0f * lightDepth), panel);
    aabbs.push_back(ceiling);
    OffsetAABB++;


    Material lightMat = Material(glm::vec3(0.0f), 0.0f, glm::vec3(16.86, 10.76, 8.2), 1.0f, glm::vec3(0.0f), 0.0f, 0.0f, 0.0f);
    GPUAABB light = GPUAABB(glm::vec3(0.0f, lightHeight, 0.0f), glm::vec3(lightWidth, EPSILON, lightDepth), lightMat);
    // aabbs.push_back(light);
    Light light_;
    light_.type = 1;
    light_.PrimitiveOffset = OffsetAABB;
    // lights.push_back(light_);
    OffsetAABB++;

    GPUSphere centre;
    centre.mat.albedo = glm::vec3(0.1f, 0.2f, 0.5f);
    spheres.push_back(centre);

    GPUSphere left;
    left.position = glm::vec3(-2.0f, 0.0f, 0.0f);
    left.mat.refractionChance = 1.0f;
    left.mat.roughness = 0.0f;
    left.mat.ior = 1.55f;
    left.mat.specularChance = 0.02f;
    left.mat.absorption = glm::vec3(1.0f, 2.0f, 3.0f);
    spheres.push_back(left);

    GPUSphere right;
    right.position = glm::vec3(2.0f, 0.0f, 0.0f);
    right.mat.albedo = { 0.8f, 0.6f, 0.2f };
    right.mat.specularChance = 1.0f;
    right.mat.metallic = 1.0f;
    right.mat.roughness = 0.1f;
    spheres.push_back(right);

    GPUSphere sun;
    sun.radius = 250.0f;
    sun.position = glm::vec3(10'000);
    sun.mat.emissive = glm::vec3(1);
    sun.mat.emissiveStrength = 0.5f;
    spheres.push_back(sun);

    Light sun_l;
    sun_l.type = 0;
    sun_l.PrimitiveOffset = spheres.size() - 1;
    lights.push_back(sun_l);
}

void Scene::CornellBox()
{
    glm::vec3 WHITECOLOR = glm::vec3(.7295, .7355, .729)*0.7f;
    glm::vec3 GREENCOLOR = glm::vec3(.117, .4125, .115)*0.7f;
    glm::vec3 REDCOLOR   = glm::vec3(.611, .0555, .062)*0.7f;

    Material whiteDiffuse = Material(WHITECOLOR, 0.01f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    Material greenDiffuse = Material(GREENCOLOR, 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    Material redDiffuse = Material(REDCOLOR, 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);

    float epsilon = 0.01f;
    float boxHeight = 30.0f;
    float boxWidth = 30.0f;
    float boxDepth = 30.0f;

    GPUAABB floor = GPUAABB(glm::vec3(0.0f,-boxHeight/2.0f, 0.0f), glm::vec3(1'000'000, epsilon, 1'000'000), whiteDiffuse);
    aabbs.push_back(floor);
    GPUAABB ceiling = GPUAABB(glm::vec3(0.0f,boxHeight/2.0f, 0.0f), glm::vec3(boxWidth, epsilon, boxDepth), whiteDiffuse);
    aabbs.push_back(ceiling);
    // Left Wall
    GPUAABB red = GPUAABB(glm::vec3(-boxWidth / 2.0f, 0.0f, 0.0f), glm::vec3(epsilon, boxHeight, boxDepth), redDiffuse);
    aabbs.push_back(red);
    // Right Wall
    GPUAABB green = GPUAABB(glm::vec3(boxWidth / 2.0f, 0.0f, 0.0f), glm::vec3(epsilon, boxHeight, boxDepth), greenDiffuse);
    aabbs.push_back(green);
    // Back Wall
    GPUAABB back = GPUAABB(glm::vec3(0.0f, 0.0f, -boxDepth / 2.0f), glm::vec3(boxWidth, boxHeight, epsilon), whiteDiffuse);
    aabbs.push_back(back);

    GPUSphere sun;
    sun.radius = 250.0f;
    sun.position = glm::vec3(10'000);
    sun.mat.emissive = glm::vec3(1);
    sun.mat.emissiveStrength = 1.0f;
    spheres.push_back(sun);

    Light sun_l;
    sun_l.type = 0;
    sun_l.PrimitiveOffset = spheres.size() - 1;
    lights.push_back(sun_l);
}

void Scene::TestPrims()
{
    glm::vec3 WHITECOLOR = glm::vec3(.7295, .7355, .729)*0.7f;
    glm::vec3 GREENCOLOR = glm::vec3(.117, .4125, .115)*0.7f;
    glm::vec3 REDCOLOR   = glm::vec3(.611, .0555, .062)*0.7f;

    Primitive s;
    s.id = 0;
    s.type = PRIM_SPHERE;
    s.position = glm::vec3(-2.0f, 2.0f, 0.0f);
    s.radius = 2.0f;
    Material redDiffuse = Material(REDCOLOR, 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    s.mat = redDiffuse;
    primitives.push_back(s);
    
    Primitive g;
    g.id = 2;
    g.type = PRIM_SPHERE;
    g.position = glm::vec3(2.0f, 2.0f, 0.0f);
    g.radius = 2.0f;
    Material greenDiff = Material(GREENCOLOR, 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    g.mat = greenDiff;
    primitives.push_back(g);

    Primitive ceil;
    ceil.id = 1;
    ceil.type = PRIM_AABB;
    ceil.position = glm::vec3(0.0f);
    ceil.dimensions = glm::vec3(10'000, 0.01, 10'000);
    Material whiteDiffuse = Material(WHITECOLOR, 0.01f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    ceil.mat = whiteDiffuse;
    primitives.push_back(ceil);
    
    AddDefaultSphere();
}
