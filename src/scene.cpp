#include "scene.h"

float Rand()
{
    return (float)(std::rand()) / (float)(RAND_MAX);
}

void Scene::CustomScene()
{
    std::srand(time(0));

    float radius = 1.0f;
    Sphere ground;
    ground.position = glm::vec3(0.0f, -1000.0f - 2.0f * radius, 0.0f);
    ground.radius = 1000.0f;
    ground.mat.albedo = glm::vec3(0.8f, 0.8f, 0.8f);
    ground.mat.specularChance = 0.0f;
    spheres.push_back(ground);

    Sphere light;
    light.position = glm::vec3(0.0f, 2.0f * radius * 1.5f, -2.0f);
    light.radius = radius;
    light.mat.emissive = glm::vec3(0.3f, 0.5f, 0.8f);
    light.mat.emissiveStrength = 300.0f;
    spheres.push_back(light);

    float secondLayerHeight = 2.0f * radius * 3.0f;
    int side = 6;
    float gap = 0.25f;

    float length = (radius * 2.0f) * (float)(side) + gap * (float)(side);
    float offset = length / 2.0f;
    float delta = length / (float)(side - 1);
    
    // Row of opaque pink
    for (int i = 0; i < side; i++)
    {
        float inc = (float)(i) / (float)(side-1);
        Sphere sphere;
        sphere.position = glm::vec3((float)i * delta - offset, 0.0f, 2.0f * radius);
        sphere.radius = radius;
        sphere.mat.albedo = glm::vec3(0.9f, 0.25f, 0.25f);
        sphere.mat.specularChance = 0.02f;
        sphere.mat.specularRoughness = 0.0f; 
        sphere.mat.refractionChance = 0.0f;
        sphere.mat.refractionRoughness = 0.0f;
        sphere.mat.ior = 1.0f + inc;
        spheres.push_back(sphere);
    }

    // Row of matte purple
    for (int i = 0; i < side; i++)
    {
        float albInc = (float)(i+1) / (float)side;
        float inc = (float)(i) / (float)(side-1);
        Sphere sphere;
        sphere.position = glm::vec3((float)i * delta - offset, secondLayerHeight, 2.0f * radius);
        sphere.radius = radius;
        sphere.mat.albedo = glm::vec3(0.82f, 0.6f, 0.95) * albInc;
        sphere.mat.specularChance = 0.16f;
        sphere.mat.specularRoughness = 1.0f - inc; 
        sphere.mat.refractionChance = 0.0f;
        sphere.mat.refractionRoughness = 0.0f;
        sphere.mat.ior = 1.0f;
        spheres.push_back(sphere);
    }
    

    // Row of glass with increasing absorption
    for (int i = 0; i < side; i++)
    {
        float inc = (float)(i) / (float)(side-1);
        Sphere sphere;
        sphere.position = glm::vec3((float)i * delta - offset, 0.0f, -2.0f * radius);
        sphere.radius = radius;
        sphere.mat.albedo = glm::vec3(0.0f, 0.0f, 0.0f);
        sphere.mat.absorption = glm::vec3(1.0f, 2.0f, 3.0f) * inc;
        sphere.mat.refractionChance = 0.96f;
        sphere.mat.refractionRoughness = 0.0f;
        sphere.mat.ior = 1.1f;
        sphere.mat.specularChance = 0.04f;
        sphere.mat.specularRoughness = 0.0f;
        spheres.push_back(sphere);
    }

    // Row of glass with increasing roughness
    for (int i = 0; i < side; i++)
    {
        float inc = (float)(i) / (float)(side-1);
        Sphere sphere;
        sphere.position = glm::vec3((float)i * delta - offset, secondLayerHeight, -2.0f * radius);
        sphere.radius = radius;
        sphere.mat.albedo = glm::vec3(0.0f, 0.0f, 0.0f);
        sphere.mat.absorption = glm::vec3(0.0f, 0.0f, 0.0f);
        sphere.mat.refractionChance = 1.0f;
        sphere.mat.refractionRoughness = inc;
        sphere.mat.ior = 1.1f;
        sphere.mat.specularChance = 0.02f;
        sphere.mat.specularRoughness = inc;
        spheres.push_back(sphere);
    }
}

void Scene::RTIW()
{
    Sphere ground_sphere;
    ground_sphere.position = glm::vec3(0.0f, -1001.0f, 0.0f);
    ground_sphere.radius = 1000.0f;
    ground_sphere.mat.albedo = glm::vec3(0.8f, 0.8f, 0.0f);
    spheres.push_back(ground_sphere);
    
    Sphere centre_sphere;
    centre_sphere.mat.albedo = glm::vec3(0.1f, 0.2f, 0.5f);
    centre_sphere.mat.specularChance = 0.16f;
    centre_sphere.mat.specularRoughness = 0.1f;
    centre_sphere.mat.ior = 1.2f;
    spheres.push_back(centre_sphere);

    Sphere left_sphere;
    left_sphere.position = glm::vec3(-2.0f, 0.0f, 0.0f);
    left_sphere.mat.refractionChance = 0.98f;
    left_sphere.mat.refractionRoughness = 0.1f;
    left_sphere.mat.ior = 1.2f;
    left_sphere.mat.specularChance = 0.02f;
    left_sphere.mat.specularRoughness = 0.1f;
    left_sphere.mat.absorption = glm::vec3(2.0f, 1.0f, 0.5f);
    spheres.push_back(left_sphere);

    Sphere right_sphere;
    right_sphere.position = glm::vec3(2.0f, 0.0f, 0.0f);
    right_sphere.mat.albedo = { 0.8f, 0.6f, 0.2f };
    right_sphere.mat.refractionChance = 0.98f;
    right_sphere.mat.refractionRoughness = 0.5f;
    right_sphere.mat.ior = 1.2f;
    right_sphere.mat.specularChance = 0.02f;
    right_sphere.mat.specularRoughness = 0.5f;
    right_sphere.mat.absorption = glm::vec3(2.0f, 1.0f, 0.5f);
    spheres.push_back(right_sphere);
}

void Scene::RandomizeBRDF()
{
    std::srand(time(0));

    // Sphere with large radius acts as a plane
    Sphere ground;
    ground.position = glm::vec3(0.0f, -1001.0f, 0.0f);
    ground.radius = 1000.0f;
    ground.mat.albedo = glm::vec3(0.8f);
    spheres.push_back(ground);

    Sphere light;
    light.position = glm::vec3(0.0f, 10.0f, 0.0f);
    light.radius = 0.5f;
    light.mat.emissive = glm::vec3(1.0f);
    light.mat.emissiveStrength = 100.0f;
    //spheres.push_back(light);

    float radius = 1.0;

    for (int a = -3; a < 2; a++) 
    {
        for (int b = -3; b < 2; b++) 
        {
            float mat_probability = Rand();
            glm::vec3 pos = glm::vec3(5 * a, 7 * Rand() + 1, 5 * b);
            {
                Sphere sphere;

                if (mat_probability < 0.2f) 
                {
                    // Emissive
                    glm::vec3 albedo = glm::vec3(Rand()*0.5+0.5, Rand()*0.5+0.5, Rand()*0.5+0.5);
                    sphere.position = pos;
                    sphere.radius = radius * 0.2f;
                    sphere.mat.albedo = albedo;
                    sphere.mat.emissive = albedo;
                    sphere.mat.emissiveStrength = 1000.0f * Rand();
                    spheres.push_back(sphere);
                } 
                else if (mat_probability < 0.2f + 0.4f) 
                {
                    // Specular
                    sphere.position = pos;
                    sphere.radius = radius;
                    sphere.mat.albedo = glm::vec3(Rand()*0.5+0.3, Rand()*0.5+0.2, Rand()*0.5+0.2);
                    sphere.mat.specularChance = Rand() * 0.16f;
                    sphere.mat.specularRoughness = Rand() * 0.2f;
                    sphere.mat.ior = 1.0f + Rand();
                    spheres.push_back(sphere);
                } 
                else 
                {
                    // Glass
                    float r = Rand() * 0.0f;
                    sphere.position = pos;
                    sphere.radius = radius;
                    sphere.mat.ior = 1.55f;
                    sphere.mat.refractionChance = 0.98f;
                    sphere.mat.refractionRoughness = r;
                    sphere.mat.absorption = glm::vec3(Rand(), Rand(), Rand());
                    sphere.mat.specularChance = 0.02f;
                    sphere.mat.specularRoughness = r;
                    spheres.push_back(sphere);
                }
            }
        }
    }

}
