#include "scene.h"

float Rand()
{
    return (float)(std::rand()) / (float)(RAND_MAX);
}

void Scene::Randomize()
{
    std::srand(time(0));
    spheres.clear();

    enum material { LAMBERTIAN, METAL, GLASS };
    // Sphere with large radius acts as a plane
    Sphere ground;
    material ground_mat = LAMBERTIAN;
    ground.position = glm::vec4(0.0f, -1000.5f, 0.0f, 1000.0f);
    ground.mat.type.x = ground_mat;
    ground.mat.albedo = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
    spheres.push_back(ground);

    // float radius = 1.0;

    // Generate 64 random spheres
    for (int a = -3; a < 3; a++) 
    {
        for (int b = -3; b < 3; b++) 
        {
            float mat_probability = Rand();
            //glm::vec4 pos = glm::vec4(a + 6 * Rand(), 6 * Rand(), b + 6 * Rand(), radius);
            glm::vec4 pos = glm::vec4(a, 6 * Rand(), b, 0.3 + 0.5 * Rand());

            //if (glm::length(glm::vec3(pos.x, pos.y, pos.z) - glm::vec3(4, 0.2, 0)) > 0.9) 
            {
                Sphere sphere;

                if (mat_probability < 0.4) 
                {
                    // diffuse
                    material s_material = LAMBERTIAN;
                    sphere.position = pos;
                    sphere.mat.type.x = s_material;
                    sphere.mat.albedo = glm::vec4(Rand(), Rand(), Rand(), 1.0f);
                    spheres.push_back(sphere);
                } 
                else if (mat_probability < 0.8) 
                {
                    // metal
                    material s_material = METAL;
                    sphere.position = pos;
                    sphere.mat.type.x = s_material;
                    sphere.mat.albedo = glm::vec4(Rand(), Rand(), Rand(), 1.0f);
                    sphere.mat.roughness = Rand();
                    spheres.push_back(sphere);
                } 
                else 
                {
                    // glass
                    material s_material = GLASS;
                    sphere.position = pos;
                    sphere.mat.type.x = s_material;
                    sphere.mat.ior = 1.55f;
                    spheres.push_back(sphere);
                }
            }
        }
    }

}