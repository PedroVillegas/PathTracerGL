#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <cstdlib>
#include <glm/glm.hpp>

#include "consoleLogger.h"
#include "shader.h"
#include "framebuffer.h"
#include "camera.h"
#include "renderer.h"
#include "scene.h"
#include "gui.h"

void SetupScene(Scene& scene);
void SetupViewportImage(const Renderer& renderer, const Scene& scene, uint& VAO, uint& VBO, uint& IBO, uint& SpheresUBO);
void RandomizeScene(Scene& scene);

int main(void) 
{
    uint ViewportWidth = 1000, ViewportHeight = 1000;
    Window window = Window("Path Tracing", 1000, 600);
    Shader PathTracerShader = Shader("src/shaders/vert.glsl", "src/shaders/path_tracer.glsl");
    Shader AccumShader = Shader("src/shaders/vert.glsl", "src/shaders/accumulation.glsl");
    Shader FinalOutputShader = Shader("src/shaders/vert.glsl", "src/shaders/final_output.glsl");
    Renderer renderer = Renderer(PathTracerShader, AccumShader, FinalOutputShader, ViewportWidth, ViewportHeight);
    Gui gui = Gui(window);
    Camera camera = Camera({0.0f, 0.0f, 3.0f}, 90.0f, 0.01f, 100.0f);
    Scene scene = Scene();

    uint VAO, VBO, IBO, SpheresUBO;
    
    SetupScene(scene);
    //RandomizeScene(scene);
    SetupViewportImage(renderer, scene, VAO, VBO, IBO, SpheresUBO);

    float last_frame = 0.0f;
    float dt = 0.0333f;
    bool vsync = true;

    while (!window.Closed())
    {
        float current_frame = glfwGetTime();

        // Input
        window.ProcessInput();
        vsync == true ? glfwSwapInterval(1) : glfwSwapInterval(0);

        gui.NewFrame();

        renderer.OnResize(ViewportWidth, ViewportHeight);
        camera.OnResize(renderer.GetViewportWidth(), renderer.GetViewportHeight());

        bool cameraIsMoving;
        if (camera.type == 0) cameraIsMoving = camera.FPS(dt, &window);
        if (camera.type == 1) cameraIsMoving = camera.Cinematic(dt, &window);

        if (cameraIsMoving) renderer.ResetSamples();

        {
            glBindBuffer(GL_UNIFORM_BUFFER, SpheresUBO); GLCall;
            int offset = 0;
            int SphereCount = scene.spheres.size();
            // Set Sphere object data
            glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &SphereCount);
            offset += sizeof(glm::vec4);
            glBufferSubData(GL_UNIFORM_BUFFER, offset, SphereCount * sizeof(Sphere), scene.spheres.data());
        }

        renderer.Render(scene, camera, VAO);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoTitleBar);

        ViewportWidth = ImGui::GetContentRegionAvail().x;
        ViewportHeight = ImGui::GetContentRegionAvail().y;

        uint image = renderer.GetViewportFramebuffer().GetTextureID();

        if (image)
            ImGui::Image((void*)(intptr_t)image, { (float)ViewportWidth, (float)ViewportHeight }, {0, 1}, {1, 0});

        ImGui::End();
        ImGui::PopStyleVar();

        gui.Render(renderer, camera, scene, vsync);

        window.Update();
        dt = current_frame - last_frame;
        last_frame = current_frame;
    }

    glDeleteVertexArrays(1, &VAO); GLCall;
    glDeleteBuffers(1, &VBO); GLCall;
    glDeleteBuffers(1, &IBO); GLCall;
    glDeleteBuffers(1, &SpheresUBO); GLCall;
}

void SetupScene(Scene& scene)
{
    // Material types: Lambertian = 0, Metal = 1, Dielectric = 2
    {
        Sphere centre_sphere;
        centre_sphere.mat.type.x = 0; 
        centre_sphere.position.w = 0.5f;
        centre_sphere.mat.albedo = { 0.1f, 0.2f, 0.5f, 1.0f };
        scene.spheres.push_back(centre_sphere);
    }
    {
        Sphere left_sphere;
        left_sphere.mat.type.x = 2;
        left_sphere.position = { -1.0f, 0.0f, 0.0f, 0.5f };
        left_sphere.mat.roughness = 0.0f;
        left_sphere.mat.ior = 1.5;
        scene.spheres.push_back(left_sphere);
    }
    {
        Sphere left_sphere;
        left_sphere.mat.type.x = 2;
        left_sphere.mat.ior = 1.55;
        left_sphere.position = { -1.0f, 0.0f, 0.0f, -0.4f };
        left_sphere.mat.roughness = 0.0f;
        scene.spheres.push_back(left_sphere);
    }
    {
        Sphere right_sphere;
        right_sphere.mat.type.x = 1;
        right_sphere.mat.albedo = { 0.8f, 0.6f, 0.2f, 1.0f };
        right_sphere.position = { 1.0f, 0.0f, 0.0f, 0.5f };
        right_sphere.mat.roughness = 0.0f;
        scene.spheres.push_back(right_sphere);
    }
    {
        Sphere ground_sphere;
        ground_sphere.mat.albedo = { 0.8f, 0.8f, 0.0f, 1.0f };
        ground_sphere.position = { 0.0f, -1000.5f, 0.0f, 1000.0f };
        scene.spheres.push_back(ground_sphere);
    }
}

void RandomizeScene(Scene& scene)
{
    enum material { LAMBERTIAN, METAL, GLASS };
    // Sphere with large radius acts as a plane
    Sphere ground;
    material ground_mat = LAMBERTIAN;
    ground.position = glm::vec4(0.0f, -1000.0f, 0.0f, 1000.0f);
    ground.mat.type.x = ground_mat;
    ground.mat.albedo = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
    scene.spheres.push_back(ground);

    // Generate 16 random spheres
    for (int a = -2; a < 2; a++) 
    {
        for (int b = -2; b < 2; b++) 
        {
            float mat_probability = std::rand();
            glm::vec3 pos = glm::vec3(a + 0.9*std::rand(), 0.2, b + 0.9*std::rand());

            if (glm::length(pos - glm::vec3(4, 0.2, 0)) > 0.9) 
            {
                Sphere sphere;

                if (mat_probability < 0.4) 
                {
                    // diffuse
                    material s_material = LAMBERTIAN;
                    sphere.mat.type.x = s_material;
                    sphere.mat.albedo = glm::vec4(std::rand(), std::rand(), std::rand(), 1.0f) * glm::vec4(std::rand(), std::rand(), std::rand(), 1.0f);
                    scene.spheres.push_back(sphere);
                } 
                else if (mat_probability < 0.8) 
                {
                    // metal
                    material s_material = METAL;
                    sphere.mat.type.x = s_material;
                    sphere.mat.albedo = glm::vec4(std::rand() * 0.5 + 0.5, std::rand() * 0.5 + 0.5, std::rand() * 0.5 + 0.5, 1.0f);
                    sphere.mat.roughness = std::rand() * 0.5;
                    scene.spheres.push_back(sphere);
                } 
                else 
                {
                    // glass
                    Sphere sphere;
                    material s_material = GLASS;
                    sphere.mat.type.x = s_material;
                    sphere.mat.ior = 1.55f;
                    scene.spheres.push_back(sphere);
                }
            }
        }
    }

}

void SetupViewportImage(const Renderer& renderer, const Scene& scene, uint& VAO, uint& VBO, uint& IBO, uint& SpheresUBO)
{
    float vertices[] = {
        // pos                 // col
        -1.0f, -1.0f, 0.0f,    0.25f, 0.52f, 0.96f,
         1.0f, -1.0f, 0.0f,    0.86f, 0.27f, 0.22f,
         1.0f,  1.0f, 0.0f,    0.96f, 0.71f,  0.0f,
        -1.0f,  1.0f, 0.0f,    0.06f, 0.62f, 0.35f
    };

    uint indices[] = {
        0, 1, 2,
        2, 3, 0
    };
 
    glGenVertexArrays  (1, &VAO); GLCall;
    glGenBuffers       (1, &VBO); GLCall;
    glGenBuffers       (1, &IBO); GLCall;
    
    glBindVertexArray  (VAO); GLCall;
    glBindBuffer       (GL_ARRAY_BUFFER, VBO); GLCall;
    glBufferData       (GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); GLCall;
    
    glBindBuffer       (GL_ELEMENT_ARRAY_BUFFER, IBO); GLCall;
    glBufferData       (GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); GLCall;

    // position attrib
    glVertexAttribPointer      (0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); GLCall;
    glEnableVertexAttribArray  (0); GLCall;

    // colour attrib  
    glVertexAttribPointer      (1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float))); GLCall;
    glEnableVertexAttribArray  (1); GLCall;

    renderer.GetShader()->Bind();
    int SphereCount = scene.spheres.size();
    
    uint block = glGetUniformBlockIndex(renderer.GetShader()->GetID(), "ObjectData"); GLCall;
    uint bind = 0;
    glUniformBlockBinding(renderer.GetShader()->GetID(), block, bind); GLCall;

    glGenBuffers(1, &SpheresUBO); GLCall;
    glBindBuffer(GL_UNIFORM_BUFFER, SpheresUBO); GLCall;
    glBindBufferBase(GL_UNIFORM_BUFFER, bind, SpheresUBO); GLCall;
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) + SphereCount * sizeof(Sphere), nullptr, GL_STATIC_DRAW); GLCall;

    {
        int offset = 0;

        // Set Sphere object data
        glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &SphereCount);
        offset += sizeof(glm::vec4);

        glBufferSubData(GL_UNIFORM_BUFFER, offset, SphereCount * sizeof(Sphere), scene.spheres.data());
    }

    renderer.GetShader()->Unbind();
}
