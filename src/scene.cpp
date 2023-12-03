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

Scene::Scene()
{
    Eye = std::make_unique<Camera>();
    Eye->Reset();
}

Scene::~Scene() {}

void Scene::SelectScene()
{
    EmptyScene();
    Eye->Reset();
    switch (SceneIdx)
    {
        case 0: RTIW(); break;
        case 1: CornellBox(); break;
        case 2: WhiteRoomColouredLights(); break;
    }
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

Material Scene::CreateLightMat(glm::vec3 le, float intensity)
{
    Material out = Material();
    out.emissive = le;
    out.intensity = intensity;

    return out;
}

void Scene::WhiteRoomColouredLights()
{
    using namespace glm;

    Eye->position   = vec3(-7.f, 25.22f, -22.54f);
    Eye->forward    = vec3(0.28f, 0.22f, 0.94f);
    Eye->RecalculateView();

    day = 0;

    float roomWidth  = 50.f;
    float roomHeight = 60.f;
    float roomDepth  = 100.0f;
    float thickness  = 1.0f;

    Material White = CreateDiffuseMat(vec3(.7295, .7355, .729)*1.1f, 1.0f);
    Material Red   = CreateDiffuseMat(vec3(.886, .102, .102)*1.0f, 1.0f);
    Material Green = CreateDiffuseMat(vec3(.102, .886, .102)*1.0f, 1.0f);
    Material Blue  = CreateDiffuseMat(vec3(.102, .5, .886)*1.0f, 1.0f);  
    
    float unit = roomWidth/7.0f;
    // Floor
    AddCube(vec3(-3.f * unit, 1.5f * unit, 0.f), vec3(unit, unit, roomDepth), White);
    AddCube(vec3(-1.f * unit, 1.5f * unit, 0.f), vec3(unit, unit, roomDepth), White);
    AddCube(vec3(+1.f * unit, 1.5f * unit, 0.f), vec3(unit, unit, roomDepth), White);
    AddCube(vec3(+3.f * unit, 1.5f * unit, 0.f), vec3(unit, unit, roomDepth), White);
    
    // Ceiling
    AddCube(vec3(0.0f, roomHeight+thickness/2.0f, 0.0f), vec3(roomWidth, 0, roomDepth)+thickness, White);

    // Walls
    AddCube(vec3(-roomWidth/2.0f, roomHeight/2.0f, 0.0f), vec3(0, roomHeight, roomDepth)+thickness, White);
    AddCube(vec3(+roomWidth/2.0f, roomHeight/2.0f, 0.0f), vec3(0, roomHeight, roomDepth)+thickness, White);
    AddCube(vec3(0.0f, roomHeight/2.0f, +roomDepth/2.0f), vec3(roomWidth, roomHeight, 0)+thickness, White);
    AddCube(vec3(0.0f, roomHeight/2.0f, -roomDepth/2.0f), vec3(roomWidth, roomHeight, 0)+thickness, White);

    AddSphere(vec3(0.f, roomHeight/2.f, 0.f), 5.0f, CreateGlassMat(vec3(0.0f), 1.55f, 0.01f));
    // AddSphere(vec3(-12.5f, 4.0f, 4.5f), 4.0f, CreateMirrorMat(vec3(1.0f), 0.1f));

    vec3 WarmSun = vec3(.992156862745098, .8862745098039216, .6862745098039216);
    vec3 LeftL   = vec3(0.012f, 0.039f, 0.51f);
    vec3 RightL  = vec3(0.51f, 0.141f, 0.012f);

    // Lights
    AddSphere(vec3(+2.f * unit, unit/2.f, roomDepth/2.f - unit), unit/2.f, CreateLightMat(LeftL, 10.f));
    AddSphere(vec3(-0.f * unit, unit/2.f, roomDepth/2.f - unit), unit/2.f, CreateLightMat(WarmSun, 1.f));
    AddSphere(vec3(-2.f * unit, unit/2.f, roomDepth/2.f - unit), unit/2.f, CreateLightMat(RightL, 12.f));

    // AddCube(vec3(+2.f * unit, 0.f, 0.f), vec3(unit, unit, roomDepth), CreateLightMat(vec3(1.f, 1.f, 5.f), 1.f));
    // AddCube(vec3(-0.f * unit, 0.f, 0.f), vec3(unit, unit, roomDepth), CreateLightMat(vec3(1.f, 5.f, 1.f), 1.f));
    // AddCube(vec3(-2.f * unit, 0.f, 0.f), vec3(unit, unit, roomDepth), CreateLightMat(vec3(5.f, 1.f, 1.f), 1.f));
    
    Init();
}

void Scene::RTIW()
{
    using namespace glm;

    Eye->position   = vec3(17.64f, 7.61f, 16.31f);
    Eye->forward    = vec3(-0.92f, -0.11f, -0.36f);
    Eye->RecalculateView();

    sunElevation    = 14.0f;
    sunAzimuth      = -200.0f;

    float roomWidth         = 50.0f;
    float roomHeight        = roomWidth/2.0f;
    float roomDepth         = 50.0f;
    float windowWidth       = roomWidth/3.0f;
    float windowHeightDim   = roomHeight/2.0f; // How tall the window is
    float windowHeightPos   = 0.5f * roomHeight;

    float thickness = 1.0f;
    Material White  = CreateDiffuseMat(vec3(.7295, .7355, .729)*1.1f, 1.0f);
    Material Red    = CreateDiffuseMat(vec3(.886, .102, .102)*1.0f, 1.0f);
    Material Green  = CreateDiffuseMat(vec3(.102, .886, .102)*1.0f, 1.0f);
    Material Blue   = CreateDiffuseMat(vec3(.102, .5, .886)*1.0f, 1.0f);  
    
    float floorStripW = roomWidth/6.0f;
    // Floor
    AddCube(vec3(-roomWidth/4.0f, -thickness/2.0f, 0.0f), vec3(roomWidth/2.0f, 0, roomDepth)+thickness, White);
    AddCube(vec3(1.0f*floorStripW/2.0f, -thickness/2.0f, 0.0f), vec3(floorStripW, 0, roomDepth)+thickness, Blue);
    AddCube(vec3(3.0f*floorStripW/2.0f, -thickness/2.0f, 0.0f), vec3(floorStripW, 0, roomDepth)+thickness, Green);
    AddCube(vec3(5.0f*floorStripW/2.0f, -thickness/2.0f, 0.0f), vec3(floorStripW, 0, roomDepth)+thickness, Red);
    
    // Ceiling
    AddCube(vec3(0.0f, roomHeight+thickness/2.0f, 0.0f), vec3(roomWidth, 0, roomDepth)+thickness, White);

    /* Wall with window */
    float w = (roomWidth - windowWidth)/2.0f;
    float h = (roomHeight - windowHeightDim);
    AddCube(vec3(-windowWidth, roomHeight/2.0f, -roomDepth/2.0f), 
            vec3(w, roomHeight, thickness),
            White);

    // Above window
    AddCube(vec3(0, roomHeight - ((1.0f - windowHeightPos/roomHeight) * h)/2.0f, -roomDepth/2.0f), 
            vec3(windowWidth, (1.0f - windowHeightPos/roomHeight) * h, 0)+thickness, 
            White);
    // Below window
    AddCube(vec3(0, ((windowHeightPos/roomHeight) * h)/2.0f, -roomDepth/2.0f), 
            vec3(windowWidth, (windowHeightPos/roomHeight) * h, 0)+thickness,
            White);
    AddCube(vec3(windowWidth, roomHeight/2.0f, -roomDepth/2.0f), 
            vec3(w, roomHeight, 0)+thickness,
            White);

    // Window Bar
    float barDim = thickness * 0.2f;
    AddCube(vec3(0.0f, roomHeight/2.0f, -roomDepth/2.0f), vec3(barDim, windowHeightDim, barDim), White);
    AddCube(vec3(0.0f, roomHeight/2.0f, -roomDepth/2.0f), vec3(windowWidth, barDim, barDim), White);

    // Walls left and right of Window resp.
    AddCube(vec3(-roomWidth/2.0f, roomHeight/2.0f, 0.0f), vec3(0, roomHeight, roomWidth)+thickness, White);
    AddCube(vec3(+roomWidth/2.0f, roomHeight/2.0f, 0.0f), vec3(0, roomHeight, roomWidth)+thickness, White);

    // Wall opposite window
    AddCube(vec3(0.0f, roomHeight/2.0f, +roomDepth/2.0f), vec3(roomWidth, roomHeight, 0)+thickness, White);

    AddSphere(vec3(-12.5f, 4.0f, 4.5f), 4.0f, CreateGlassMat(vec3(0.0f), 1.55f, 0.01f));
    // AddSphere(vec3(-12.5f, 4.0f, 4.5f), 4.0f, CreateMirrorMat(vec3(1.0f), 0.1f));
    
    Init();
}

void Scene::CornellBox()
{
    using namespace glm;

    Eye->position   = vec3(0.f, 5.f, 10.f);
    Eye->forward    = vec3(0.f, 0.f, -1.f);
    Eye->RecalculateView();

    vec3 WHITE_COL = vec3(.7295, .7355, .729)*1.0f;
    vec3 RED_COL   = vec3(.611, .0555, .062)*0.7f;
    vec3 GREEN_COL = vec3(.117, .4125, .115)*0.7f;
    vec3 BLUE_COL  = vec3(0.08f, 0.16f, 0.29f);

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
    AddCube(vec3(0.0f), vec3(10.f, 0.01f, 10.f), whiteDiffuse);

    Material GLASS = CreateGlassMat(vec3(0.3f, 0.2f, 0.1f), 1.55f, 0.1f);
    AddSphere(vec3(-2.0f, 2.5f, -2.0f), 2.0f, GLASS);

    Material MIRROR = CreateMirrorMat(vec3(0.7f), 0.1f);
    // AddSphere(vec3(2.0f, 7.0f, 2.0f), 2.0f, MIRROR);
    // AddCube(vec3(0.0f, 2.0f, 0.0f), vec3(4.0f), MIRROR);

    Material light = CreateLightMat(vec3(.992156862745098f, .8862745098039216f, .6862745098039216f), 10.f);
    // AddSphere(vec3(0.0f, 8.0f, 0.0f), 1.0f, light);
    AddCube(vec3(0.0f, 10.0f - 0.01f, 0.0f), vec3(2.5f, 0.01f, 2.5f), light);

    Init();
}
