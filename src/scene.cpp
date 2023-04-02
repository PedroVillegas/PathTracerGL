#include "scene.h"

float Rand()
{
    return (float)(std::rand()) / (float)(RAND_MAX);
}

void Scene::emptyScene()
{
    spheres.clear();
    aabbs.clear();
    lights.clear();
}

void Scene::GridShowcase()
{
    std::srand(time(0));

    float floorThickness = 0.5f;
    Material floorMat = Material(glm::vec3(1.0f, 0.95f, 0.8f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    GPUAABB floor = GPUAABB(
        glm::vec3(0.0f, -(1.0f + 0.5f * floorThickness), 0.0f),
        glm::vec3(50.0f, floorThickness, 50.0f), floorMat);
    aabbs.push_back(floor);

    float EPSILON = 1e-3f;
    float lightHeight = 8.0f;
    float lightWidth = 4.0f;
    float lightDepth = 1.0f;
    float panelThickness = 0.1f;

    Material panel = Material(glm::vec3(0.5f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    Material lightMat = Material(glm::vec3(0.0f), 0.0f, glm::vec3(16.86, 10.76, 8.2), 200.0f, glm::vec3(0.0f), 0.0f, 0.0f, 0.0f);

    GPUAABB ceiling1 = GPUAABB(
        glm::vec3(0.0f, lightHeight + 0.5f * panelThickness + EPSILON, 0.0f),
        glm::vec3(1.25f * lightWidth, panelThickness, 2.0f * lightDepth), panel);
    aabbs.push_back(ceiling1);

    GPUAABB light1 = GPUAABB(glm::vec3(0.0f, lightHeight, 0.0f), glm::vec3(lightWidth, EPSILON, lightDepth), lightMat);
    aabbs.push_back(light1);

    GPUAABB ceiling2 = GPUAABB(
        glm::vec3(0.0f, lightHeight + 0.5f * panelThickness + EPSILON, -2.5f),
        glm::vec3(1.25f * lightWidth, panelThickness, 2.0f * lightDepth), panel);
    aabbs.push_back(ceiling2);

    GPUAABB light2 = GPUAABB(glm::vec3(0.0f, lightHeight, -2.5f), glm::vec3(lightWidth, EPSILON, lightDepth), lightMat);
    aabbs.push_back(light2);

    GPUAABB ceiling3 = GPUAABB(
        glm::vec3(0.0f, lightHeight + 0.5f * panelThickness + EPSILON, 2.5f),
        glm::vec3(1.25f * lightWidth, panelThickness, 2.0f * lightDepth), panel);
    aabbs.push_back(ceiling3);

    GPUAABB light3 = GPUAABB(glm::vec3(0.0f, lightHeight, 2.5f), glm::vec3(lightWidth, EPSILON, lightDepth), lightMat);
    aabbs.push_back(light3);

    float radius = 1.0f;
    int side = 4;
    float gap = 0.15f;
    float length = (radius * 2.0f) * (float)(side) + gap * (float)(side);
    float offset = length / 2.0f;
    float delta = length / (float)(side - 1);
    int idOffset = 2;
    
    for (int i = 0; i < side; i++)
    {
        float inc = (float)(i) / (float)(side-1);
        for (int j = 0; j < side; j++)
        {
            Sphere sp;
            sp.label = "Sphere " + std::to_string(i);
            GPUSphere& s = sp.sphere;
            s.geomID = idOffset;
            s.position = glm::vec3((float)i * delta - offset, 0.0f, (float)j * delta - offset);
            s.radius = radius;
            s.mat.albedo = glm::vec3(0.9f, 0.25f, 0.25f);
            s.mat.specularChance = 0.04f;
            s.mat.roughness = 0.0f;
            s.mat.ior = 1.0f + inc;
            spheres.push_back(sp);
            idOffset++;
        }
    }
}

void Scene::CustomScene()
{
    std::srand(time(0));
    float radius = 1.0f;

    float floorThickness = 0.5f;
    Material floorMat = Material(glm::vec3(1.0f, 0.95f, 0.8f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    GPUAABB floor = GPUAABB(
        glm::vec3(0.0f, -(1.0f + 0.5f * floorThickness), 0.0f),
        glm::vec3(50.0f, floorThickness, 50.0f), floorMat);
    aabbs.push_back(floor);

    float EPSILON = 1e-3f;
    float lightHeight = 8.0f;
    float lightWidth = 4.0f;
    float lightDepth = 1.0f;
    float panelThickness = 0.1f;

    Material panel = Material(glm::vec3(0.5f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    GPUAABB ceiling = GPUAABB(
        glm::vec3(0.0f, lightHeight + 0.5f * panelThickness + EPSILON, 0.0f),
        glm::vec3(1.25f * lightWidth, panelThickness, 2.0f * lightDepth), panel);
    aabbs.push_back(ceiling);

    Material lightMat = Material(glm::vec3(0.0f), 0.0f, glm::vec3(16.86, 10.76, 8.2), 1.0f, glm::vec3(0.0f), 0.0f, 0.0f, 0.0f);
    GPUAABB light = GPUAABB(glm::vec3(0.0f, lightHeight, 0.0f), glm::vec3(lightWidth, EPSILON, lightDepth), lightMat);
    aabbs.push_back(light);

    float secondLayerHeight = 3.0f * radius;
    int side = 5;
    float gap = 0.2f;

    float length = (radius * 2.0f) * (float)(side) + gap * (float)(side);
    float offset = length / 2.0f;
    float delta = length / (float)(side - 1);
    
    int idOffset = 2;
    
    // Row of opaque pink
    for (int i = 0; i < side; i++)
    {
        float inc = (float)(i) / (float)(side-1);
        Sphere sphere;
        sphere.label = "Pink Sphere " + std::to_string(i);
        GPUSphere& s = sphere.sphere;
        s.geomID = idOffset;
        s.position = glm::vec3((float)i * delta - offset, 0.0f, 2.0f * radius);
        s.radius = radius;
        s.mat.albedo = glm::vec3(0.9f, 0.25f, 0.25f);
        s.mat.specularChance = 0.06f;
        s.mat.refractionChance = 0.0f;
        s.mat.roughness = 0.0f;
        s.mat.ior = 1.0f + inc;
        spheres.push_back(sphere);
        idOffset++;
    }

    // Row of matte purple
    for (int i = 0; i < side; i++)
    {
        float albInc = (float)(i+1) / (float)side;
        float inc = (float)(i) / (float)(side-1);
        Sphere sphere;
        sphere.label = "Purple Sphere " + std::to_string(i);
        GPUSphere& s = sphere.sphere;
        s.geomID = idOffset;
        s.position = glm::vec3((float)i * delta - offset, secondLayerHeight, 2.0f * radius);
        s.radius = radius;
        s.mat.albedo = glm::vec3(0.82f, 0.6f, 0.95) * albInc;
        s.mat.specularChance = 0.16f;
        s.mat.roughness = 1.0f - inc; 
        s.mat.refractionChance = 0.0f;
        s.mat.ior = 1.0f;
        spheres.push_back(sphere);
        idOffset++;
    }
    

    // Row of glass with increasing absorption
    for (int i = 0; i < side; i++)
    {
        float inc = (float)(i) / (float)(side-1);
        Sphere sphere;
        sphere.label = "Absorbed Glass " + std::to_string(i);
        GPUSphere& s = sphere.sphere; 
        s.geomID = idOffset;
        s.position = glm::vec3((float)i * delta - offset, 0.0f, -2.0f * radius);
        s.radius = radius;
        s.mat.albedo = glm::vec3(1.0f, 1.0f, 1.0f);
        s.mat.absorption = glm::vec3(1.0f, 2.0f, 3.0f) * inc;
        s.mat.refractionChance = 0.98f;
        s.mat.ior = 1.55f;
        s.mat.specularChance = 0.02f;
        s.mat.roughness = 0.0f;
        spheres.push_back(sphere);
        idOffset++;
    }

    // Row of glass with increasing roughness
    for (int i = 0; i < side; i++)
    {
        float inc = (float)(i) / (float)(side-1);
        Sphere sphere;
        sphere.label = "Rough Glass " + std::to_string(i);
        GPUSphere& s = sphere.sphere;
        s.geomID = idOffset;
        s.position = glm::vec3((float)i * delta - offset, secondLayerHeight, -2.0f * radius);
        s.radius = radius;
        s.mat.albedo = glm::vec3(1.0f, 1.0f, 1.0f);
        s.mat.absorption = glm::vec3(0.0f, 0.0f, 0.0f);
        s.mat.refractionChance = 0.98f;
        s.mat.roughness = inc;
        s.mat.ior = 1.55f;
        s.mat.specularChance = 0.02f;
        spheres.push_back(sphere);
        idOffset++;
    }
}

void Scene::RTIW()
{
    float floorThickness = 0.5f;
    Material floorMat = Material(glm::vec3(1.0f, 0.95f, 0.8f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    GPUAABB floor = GPUAABB(
        glm::vec3(0.0f, -(1.0f + 0.5f * floorThickness), 0.0f),
        glm::vec3(50.0f, floorThickness, 50.0f), floorMat);
    aabbs.push_back(floor);

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

    Material lightMat = Material(glm::vec3(0.0f), 0.0f, glm::vec3(16.86, 10.76, 8.2), 1.0f, glm::vec3(0.0f), 0.0f, 0.0f, 0.0f);
    GPUAABB light = GPUAABB(glm::vec3(0.0f, lightHeight, 0.0f), glm::vec3(lightWidth, EPSILON, lightDepth), lightMat);
    aabbs.push_back(light);
    
    Sphere slight;
    slight.label = "Light";
    GPUSphere& l = slight.sphere;
    l.geomID = 1;
    l.position = glm::vec3(0.0f, 4.0f, 0.0f);
    l.radius = 1.0f;
    l.mat.emissive = glm::vec3(16.86, 10.76, 8.2);
    l.mat.emissiveStrength = 1.0f;
    // spheres.push_back(slight);
    // lights.push_back(slight.GetLight());

    Sphere centre_sphere;
    centre_sphere.label = "Centre";
    GPUSphere& s_centre = centre_sphere.sphere;
    s_centre.geomID = 2;
    s_centre.mat.albedo = glm::vec3(0.1f, 0.2f, 0.5f);
    spheres.push_back(centre_sphere);

    Sphere left_sphere;
    left_sphere.label = "Left";
    GPUSphere& s_left = left_sphere.sphere;
    s_left.geomID = 3;
    s_left.position = glm::vec3(-2.0f, 0.0f, 0.0f);
    s_left.mat.refractionChance = 1.0f;
    s_left.mat.roughness = 0.0f;
    s_left.mat.ior = 1.55f;
    s_left.mat.specularChance = 0.02f;
    s_left.mat.absorption = glm::vec3(1.0f, 2.0f, 3.0f);
    spheres.push_back(left_sphere);

    Sphere right_sphere;
    right_sphere.label = "Right";
    GPUSphere& s_right = right_sphere.sphere;
    s_right.geomID = 4;
    s_right.position = glm::vec3(2.0f, 0.0f, 0.0f);
    s_right.mat.albedo = { 0.8f, 0.6f, 0.2f };
    s_right.mat.specularChance = 1.0f;
    s_right.mat.metallic = 1.0f;
    s_right.mat.roughness = 0.0f;
    spheres.push_back(right_sphere);
}

void Scene::ModifiedCornellBox()
{

    glm::vec3 WHITECOLOR = glm::vec3(.7295, .7355, .729)*0.7f;
    glm::vec3 GREENCOLOR = glm::vec3(.117, .4125, .115)*0.7f;
    glm::vec3 REDCOLOR   = glm::vec3(.611, .0555, .062)*0.7f;

    Material whiteDiffuse = Material(WHITECOLOR, 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    Material greenDiffuse = Material(GREENCOLOR, 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    Material redDiffuse = Material(REDCOLOR, 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    Material glass = Material(glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.1f), 1.0f, 1.55f, 0.0f);
    
    float epsilon = 0.01f;
    float boxHeight = 20.0f;
    float boxWidth = 30.0f;
    float boxDepth = 30.0f;

    GPUAABB floor = GPUAABB(glm::vec3(0.0f, -boxHeight / 2.0f, 0.0f), glm::vec3(boxWidth, epsilon, boxDepth), whiteDiffuse);
    aabbs.push_back(floor);
    GPUAABB ceiling = GPUAABB(glm::vec3(0.0f, boxHeight / 2.0f, boxDepth / 8.0f), glm::vec3(boxWidth, epsilon, 3.0f * boxDepth / 4.0f), whiteDiffuse);
    aabbs.push_back(ceiling);
    // Left Wall
    GPUAABB red = GPUAABB(glm::vec3(-boxWidth / 2.0f, boxHeight / 4.0f, 0.0f), glm::vec3(epsilon, 2.0f * boxHeight, boxDepth), redDiffuse);
    aabbs.push_back(red);
    // Right Wall
    GPUAABB green = GPUAABB(glm::vec3(boxWidth / 2.0f, boxHeight / 4.0f, 0.0f), glm::vec3(epsilon, 2.0f * boxHeight, boxDepth), greenDiffuse);
    aabbs.push_back(green);
    // Back Wall
    GPUAABB back = GPUAABB(glm::vec3(0.0f, boxHeight / 4.0f, -boxDepth / 2.0f), glm::vec3(boxWidth, 2.0f * boxHeight, epsilon), whiteDiffuse);
    aabbs.push_back(back);
    // Front Wall
    GPUAABB front = GPUAABB(glm::vec3(0.0f, 0.0f, boxDepth / 2.0f), glm::vec3(boxWidth, 2.0f * boxHeight, epsilon), whiteDiffuse);
    aabbs.push_back(front);

    Sphere light;
    light.label = "Light";
    GPUSphere& l = light.sphere;
    l.radius = 50.0f;
    l.position = glm::vec3(0.0f, boxHeight * 2.0f, 0.0f);
    l.mat.emissive = glm::vec3(16.86f, 10.76f, 8.2f) * 200.0f;
    // spheres.push_back(light);

    Sphere sp1;
    sp1.label = "Sphera 1";
    GPUSphere& s1 = sp1.sphere;
    s1.geomID = 2;
    s1.radius = 3.0f;
    s1.position = glm::vec3(
                (-boxWidth / 2.0f) + 0.25f * boxWidth,
                (-boxHeight / 2.0f) + 0.2f * boxHeight,
                (-boxDepth / 2.0f) + 0.25f * boxDepth) + glm::vec3(s1.radius);
    s1.mat.absorption = glm::vec3(0.0f, 0.5f, 1.0f);
    s1.mat.ior = 1.55f;
    s1.mat.refractionChance = 1.0f;
    s1.mat.specularChance = 0.02f;
    s1.mat.roughness = 0.0f;
    spheres.push_back(sp1);
    
    Sphere sp2;
    sp2.label = "Sphera 2";
    GPUSphere& s2 = sp2.sphere;
    s2.geomID = 2;
    s2.radius = 3.0f;
    s2.position = glm::vec3(
                (boxWidth / 2.0f) - 0.25f * boxWidth,
                (boxHeight / 2.0f) - 0.2f * boxHeight,
                (boxDepth / 2.0f) - 0.5f * boxDepth) - glm::vec3(s2.radius);
    s2.mat.albedo = glm::vec3(0.6f);
    s2.mat.specularChance = 1.0f;
    s2.mat.metallic = 1.0f;
    s2.mat.roughness = 0.0f;
    spheres.push_back(sp2);
}

void Scene::CornellBox()
{
    glm::vec3 WHITECOLOR = glm::vec3(.7295, .7355, .729)*0.7f;
    glm::vec3 GREENCOLOR = glm::vec3(.117, .4125, .115)*0.7f;
    glm::vec3 REDCOLOR   = glm::vec3(.611, .0555, .062)*0.7f;

    Material whiteDiffuse = Material(WHITECOLOR, 0.01f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    Material greenDiffuse = Material(GREENCOLOR, 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    Material redDiffuse = Material(REDCOLOR, 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 1.0f, 1.0f);
    Material glass = Material(glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, glm::vec3(0.1f), 1.0f, 1.55f, 0.0f);
    
    float epsilon = 0.5f;
    float boxHeight = 20.0f;
    float boxWidth = 30.0f;
    float boxDepth = 30.0f;

    GPUAABB floor = GPUAABB(glm::vec3(0.0f, -boxHeight / 2.0f, 0.0f), glm::vec3(boxWidth, epsilon, boxDepth), whiteDiffuse);
    aabbs.push_back(floor);
    GPUAABB ceiling = GPUAABB(glm::vec3(0.0f, boxHeight / 2.0f, 0.0f), glm::vec3(boxWidth, epsilon, boxDepth), whiteDiffuse);
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
    // Front Wall
    // GPUAABB front = GPUAABB(glm::vec3(0.0f, 0.0f, boxDepth / 2.0f), glm::vec3(boxWidth, boxHeight, epsilon), whiteDiffuse);
    // aabbs.push_back(front);
    // Slightly open front wall
    GPUAABB front = GPUAABB(glm::vec3(0.0f, -boxHeight / 6.0f, boxDepth / 2.0f), glm::vec3(boxWidth, 2.0f * boxHeight / 3.0f, 1.0f), whiteDiffuse);
    aabbs.push_back(front);
    
    float lightWidth = boxWidth * 0.3334;
    float lightDepth = boxDepth * 0.3334;
    Material lightMat = Material(glm::vec3(0.0f), 0.0f, glm::vec3(16.86, 10.76, 8.2), 1.0f, glm::vec3(0.0f), 0.0f, 0.0f, 0.0f);
    GPUAABB light = GPUAABB(glm::vec3(0.0f, (boxHeight / 2.0f) - epsilon / 2.0f, 0.0f), glm::vec3(lightWidth, 0.01f, lightDepth), lightMat);
    aabbs.push_back(light);

    Sphere sp1;
    sp1.label = "Sphera 1";
    GPUSphere& s1 = sp1.sphere;
    s1.geomID = 2;
    s1.radius = 3.0f;
    s1.position = glm::vec3(
                (-boxWidth / 2.0f) + 0.25f * boxWidth,
                (-boxHeight / 2.0f) + 0.2f * boxHeight,
                (-boxDepth / 2.0f) + 0.25f * boxDepth) + glm::vec3(s1.radius);
    s1.mat.absorption = glm::vec3(1.0f, 0.3f, 0.0f);
    s1.mat.ior = 1.55f;
    s1.mat.refractionChance = 1.0f;
    s1.mat.specularChance = 0.02f;
    s1.mat.roughness = 0.0f;
    spheres.push_back(sp1);
    
    Sphere sp2;
    sp2.label = "Sphera 2";
    GPUSphere& s2 = sp2.sphere;
    s2.geomID = 2;
    s2.radius = 3.0f;
    s2.position = glm::vec3(
                (boxWidth / 2.0f) - 0.25f * boxWidth,
                (boxHeight / 2.0f) - 0.2f * boxHeight,
                (boxDepth / 2.0f) - 0.5f * boxDepth) - glm::vec3(s2.radius);
    s2.mat.albedo = glm::vec3(0.6f);
    s2.mat.specularChance = 1.0f;
    s2.mat.metallic = 1.0f;
    s2.mat.roughness = 0.0f;
    spheres.push_back(sp2);
}

void Scene::RandomizeBRDF()
{
    std::srand(time(0));

    // Sphere with large radius acts as a plane
    Sphere ground;
    ground.label = "Ground";
    GPUSphere& s_ground = ground.sphere;
    s_ground.position = glm::vec3(0.0f, -1001.0f, 0.0f);
    s_ground.radius = 1000.0f;
    s_ground.mat.albedo = glm::vec3(0.8f);
    spheres.push_back(ground);

    Sphere light;
    light.label = "Light";
    GPUSphere& l = light.sphere;
    l.position = glm::vec3(0.0f, 10.0f, 0.0f);
    l.radius = 0.5f;
    l.mat.emissive = glm::vec3(1.0f);
    l.mat.emissiveStrength = 100.0f;
    //spheres.push_back(light);

    float radius = 1.0;
    int id = 0;
    for (int a = -3; a < 2; a++) 
    {
        for (int b = -3; b < 2; b++) 
        {
            float mat_probability = Rand();
            glm::vec3 pos = glm::vec3(5 * a, 7 * Rand() + 1, 5 * b);
            {
                Sphere sphere;
                GPUSphere& s = sphere.sphere;
                s.geomID = id;

                if (mat_probability < 0.2f) 
                {
                    // Emissive
                    sphere.label = "Light" + std::to_string(std::abs(a * b));
                    glm::vec3 albedo = glm::vec3(Rand()*0.5+0.5, Rand()*0.5+0.5, Rand()*0.5+0.5);
                    s.position = pos;
                    s.radius = radius;
                    s.mat.emissive = albedo;
                    s.mat.emissiveStrength = 50.0f;
                    spheres.push_back(sphere);
                    lights.push_back(sphere.GetLight());

                } 
                else if (mat_probability < 0.2f + 0.4f) 
                {
                    // Specular
                    sphere.label = "Specular" + std::to_string(std::abs(a * b));
                    s.position = pos;
                    s.radius = radius;
                    s.mat.albedo = glm::vec3(Rand()*0.5+0.3, Rand()*0.5+0.2, Rand()*0.5+0.2);
                    s.mat.specularChance = Rand() * 0.16f;
                    s.mat.roughness = Rand() * 0.2f;
                    s.mat.ior = 1.0f + Rand();
                    spheres.push_back(sphere);
                } 
                else 
                {
                    // Glass
                    sphere.label = "Glass" + std::to_string(std::abs(a * b));
                    float r = Rand() * 0.0f;
                    s.position = pos;
                    s.radius = radius;
                    s.mat.ior = 1.55f;
                    s.mat.refractionChance = 0.98f;
                    s.mat.absorption = glm::vec3(Rand(), Rand(), Rand()) * 2.0f;
                    s.mat.specularChance = 0.02f;
                    s.mat.roughness = r;
                    spheres.push_back(sphere);
                }
            }
            id++;
        }
        id++;
    }

}
