#include "scene.h"

float Rand()
{
    return (float)(std::rand()) / (float)(RAND_MAX);
}

void Scene::CircleScene()
{
    std::srand(time(0));

    float radius = 1.0f;
    Sphere ground;
    ground.label = "Ground";
    GPUSphere& s_ground = ground.sphere;
    s_ground.position = glm::vec3(0.0f, -1000.0f - 2.0f * radius, 0.0f);
    s_ground.radius = 1000.0f;
    s_ground.mat.albedo = glm::vec3(0.8f, 0.8f, 0.8f);
    s_ground.mat.specularChance = 0.0f;
    spheres.push_back(ground);

    Sphere light;
    light.label = "Light";
    GPUSphere& s_light = light.sphere;
    s_light.position = glm::vec3(0.0f, 2.0f * radius * 1.5f, 0.0f);
    s_light.radius = radius;
    s_light.mat.emissive = glm::vec3(0.3f, 0.5f, 0.8f);
    s_light.mat.emissiveStrength = 50.0f;
    spheres.push_back(light);

    int total = 16;
    float circleRadius = 8.0f;
    for (int i = 0; i < total; i++)
    {
        float offset = (2.0f * M_PI) * ((float)i / (float)total);
        float inc = (float)i / (float)(total - 1);
        Sphere sp;
        sp.label = "Sphere" + std::to_string(i);
        GPUSphere& s = sp.sphere;
        s.position = circleRadius * glm::vec3(glm::cos(offset), 0.0f, glm::sin(offset));
        s.radius = radius;
        s.mat.absorption = glm::vec3(1.0f, 2.0f, 3.0f) * inc;
        s.mat.refractionChance = 0.98f;
        s.mat.roughness = 0.0f;
        s.mat.ior = 1.55f;
        s.mat.specularChance = 0.02f;
        spheres.push_back(sp);
    }
}

void Scene::CustomScene()
{
    std::srand(time(0));

    float radius = 1.0f;
    Sphere ground;
    ground.label = "Ground";
    GPUSphere& s_ground = ground.sphere;
    s_ground.position = glm::vec3(0.0f, -1000.0f - 2.0f * radius, 0.0f);
    s_ground.radius = 1000.0f;
    s_ground.mat.albedo = glm::vec3(0.8f, 0.8f, 0.8f);
    s_ground.mat.specularChance = 0.0f;
    spheres.push_back(ground);

    Sphere light;
    light.label = "Light";
    GPUSphere& s_light = light.sphere;
    s_light.position = glm::vec3(0.0f, 2.0f * radius * 1.5f, -2.0f);
    s_light.radius = radius;
    s_light.mat.emissive = glm::vec3(0.3f, 0.5f, 0.8f);
    s_light.mat.emissiveStrength = 50.0f;
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
        sphere.label = "Pink Sphere " + std::to_string(i);
        GPUSphere& s = sphere.sphere;
        s.position = glm::vec3((float)i * delta - offset, 0.0f, 2.0f * radius);
        s.radius = radius;
        s.mat.albedo = glm::vec3(0.9f, 0.25f, 0.25f);
        s.mat.specularChance = 0.06f;
        s.mat.refractionChance = 0.0f;
        s.mat.roughness = 0.0f;
        s.mat.ior = 1.0f + inc;
        spheres.push_back(sphere);
    }

    // Row of matte purple
    for (int i = 0; i < side; i++)
    {
        float albInc = (float)(i+1) / (float)side;
        float inc = (float)(i) / (float)(side-1);
        Sphere sphere;
        sphere.label = "Purple Sphere " + std::to_string(i);
        GPUSphere& s = sphere.sphere;
        s.position = glm::vec3((float)i * delta - offset, secondLayerHeight, 2.0f * radius);
        s.radius = radius;
        s.mat.albedo = glm::vec3(0.82f, 0.6f, 0.95) * albInc;
        s.mat.specularChance = 0.16f;
        s.mat.roughness = 1.0f - inc; 
        s.mat.refractionChance = 0.0f;
        s.mat.ior = 1.0f;
        spheres.push_back(sphere);
    }
    

    // Row of glass with increasing absorption
    for (int i = 0; i < side; i++)
    {
        float inc = (float)(i) / (float)(side-1);
        Sphere sphere;
        sphere.label = "Absorbed Glass " + std::to_string(i);
        GPUSphere& s = sphere.sphere; 
        s.position = glm::vec3((float)i * delta - offset, 0.0f, -2.0f * radius);
        s.radius = radius;
        s.mat.albedo = glm::vec3(1.0f, 1.0f, 1.0f);
        s.mat.absorption = glm::vec3(1.0f, 2.0f, 3.0f) * inc;
        s.mat.refractionChance = 0.9f;
        s.mat.ior = 1.55f;
        s.mat.specularChance = 0.1f;
        s.mat.roughness = 0.0f;
        spheres.push_back(sphere);
    }

    // Row of glass with increasing roughness
    for (int i = 0; i < side; i++)
    {
        float inc = (float)(i) / (float)(side-1);
        Sphere sphere;
        sphere.label = "Rough Glass " + std::to_string(i);
        GPUSphere& s = sphere.sphere;
        s.position = glm::vec3((float)i * delta - offset, secondLayerHeight, -2.0f * radius);
        s.radius = radius;
        s.mat.albedo = glm::vec3(1.0f, 1.0f, 1.0f);
        s.mat.absorption = glm::vec3(0.0f, 0.0f, 0.0f);
        s.mat.refractionChance = 0.98f;
        s.mat.roughness = inc;
        s.mat.ior = 1.55f;
        s.mat.specularChance = 0.02f;
        spheres.push_back(sphere);
    }
}

void Scene::RTIW()
{
    Sphere ground_sphere;
    ground_sphere.label = "Ground";
    GPUSphere& s_ground = ground_sphere.sphere;
    s_ground.position = glm::vec3(0.0f, -1001.0f, 0.0f);
    s_ground.radius = 1000.0f;
    s_ground.mat.albedo = glm::vec3(0.8f, 0.8f, 0.0f);
    spheres.push_back(ground_sphere);
    
    Sphere light;
    light.label = "Light";
    GPUSphere& l = light.sphere;
    l.position = glm::vec3(0.0f, 4.0f, 0.0f);
    l.radius = 1.0f;
    l.mat.emissive = glm::vec3(0.1f, 0.2f, 0.5f);
    l.mat.emissiveStrength = 10.0f;
    spheres.push_back(light);

    Sphere centre_sphere;
    centre_sphere.label = "Centre";
    GPUSphere& s_centre = centre_sphere.sphere;
    s_centre.mat.albedo = glm::vec3(0.1f, 0.2f, 0.5f);
    s_centre.mat.specularTint = glm::vec3(1.0f);
    s_centre.mat.specularChance = 0.16f;
    s_centre.mat.roughness = 0.02f;
    s_centre.mat.ior = 1.3f;
    spheres.push_back(centre_sphere);

    Sphere left_sphere;
    left_sphere.label = "Left";
    GPUSphere& s_left = left_sphere.sphere;
    s_left.position = glm::vec3(-2.0f, 0.0f, 0.0f);
    s_left.mat.refractionChance = 0.98f;
    s_left.mat.roughness = 0.02f;
    s_left.mat.ior = 1.55f;
    s_left.mat.specularTint = glm::vec3(1.0f);
    s_left.mat.specularChance = 0.02f;
    s_left.mat.absorption = glm::vec3(2.0f, 1.0f, 0.5f);
    spheres.push_back(left_sphere);

    Sphere right_sphere;
    right_sphere.label = "Right";
    GPUSphere& s_right = right_sphere.sphere;
    s_right.position = glm::vec3(2.0f, 0.0f, 0.0f);
    s_right.mat.albedo = { 0.8f, 0.6f, 0.2f };
    s_right.mat.refractionChance = 0.98f;
    s_right.mat.ior = 1.55f;
    s_right.mat.specularTint = glm::vec3(1.0f);
    s_right.mat.specularChance = 0.02f;
    s_right.mat.roughness = 0.02f;
    s_right.mat.absorption = glm::vec3(2.0f, 1.0f, 0.5f);
    spheres.push_back(right_sphere);
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

    for (int a = -3; a < 2; a++) 
    {
        for (int b = -3; b < 2; b++) 
        {
            float mat_probability = Rand();
            glm::vec3 pos = glm::vec3(5 * a, 7 * Rand() + 1, 5 * b);
            {
                Sphere sphere;
                GPUSphere& s = sphere.sphere;

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
                    s.mat.absorption = glm::vec3(Rand() * 5.0f, Rand() * 5.0f, Rand() * 5.0f);
                    s.mat.specularChance = 0.02f;
                    s.mat.roughness = r;
                    spheres.push_back(sphere);
                }
            }
        }
    }

}
