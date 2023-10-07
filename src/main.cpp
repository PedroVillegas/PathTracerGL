#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "window.h"
#include "shader.h"
#include "framebuffer.h"
#include "camera.h"
#include "renderer.h"
#include "scene.h"
#include "gui.h"
#include "bvh.h"
#include "primitives.h"

void SetupQuad(uint32_t& VAO, uint32_t& VBO, uint32_t& IBO);

int main(void) 
{
    const char* Title = "Path Tracing";
    uint32_t ViewportHeight = 720;
    uint32_t ViewportWidth = ViewportHeight * 16/9;
    Window window = Window(Title, ViewportWidth, ViewportHeight);
    Gui gui = Gui(window);
    Scene scene = Scene();
    Camera camera = Camera({0.0f, 2.0f, 6.0f}, 60.0f, 0.01f, 100.0f);
    std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>(ViewportWidth, ViewportHeight, &scene, &camera);

    uint32_t VAO, VBO, IBO, BVH_UBO;

    // Unit Cube centered about the origin
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f,
        -0.5f,  0.5f, -0.5f, 1.0f,
        -0.5f, -0.5f,  0.5f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f,
        -0.5f,  0.5f,  0.5f, 1.0f,
    };

    uint32_t elements[] = {
        0, 1, 2, 3,
        4, 5, 6, 7,
        0, 4, 1, 5, 
        2, 6, 3, 7
    };

    uint32_t debug_vao;
    glGenVertexArrays(1, &debug_vao); 
    glBindVertexArray(debug_vao); 

    uint32_t vbo_vertices;
    glGenBuffers(1, &vbo_vertices); 
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 

    uint32_t ibo_elements;
    glGenBuffers(1, &ibo_elements); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW); 

    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); 

    renderer->debugVAO = debug_vao;

    SetupQuad(VAO, VBO, IBO);

    float dt = 0.0f;
    bool vsync = true;

    while (!window.Closed())
    {
        // Input
        window.ProcessInput();
        vsync == true ? glfwSwapInterval(1) : glfwSwapInterval(0);

        gui.NewFrame();

        renderer->OnResize(ViewportWidth, ViewportHeight);
        camera.OnResize(renderer->GetViewportWidth(), renderer->GetViewportHeight());

        if (camera.OnUpdate(dt, &window)) 
            renderer->ResetSamples();
        
        if (!renderer->b_Pause)
        {
            using namespace glm;
            float phi = radians(scene.sunElevation);
            float theta = radians(scene.sunAzimuth);
            float x = sin(theta) * cos(phi);
            float y = sin(phi);
            float z = cos(theta) * cos(phi);
            scene.Data.SunDirection = glm::vec3(x,y,z);
            scene.Data.Depth = scene.maxRayDepth;
            scene.Data.SelectedPrimIdx = scene.PrimitiveIdx;
            scene.Data.Day = scene.day;

            renderer->Render(scene, camera, VAO);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoTitleBar);

        ViewportWidth = ImGui::GetContentRegionAvail().x;
        ViewportHeight = ImGui::GetContentRegionAvail().y;

        uint32_t image = renderer->GetViewportFramebuffer().GetTextureID();
        ImGui::Image((void*)(intptr_t)image, {(float)ViewportWidth, (float)ViewportHeight}, {0, 1}, {1, 0});

        ImGui::End();
        ImGui::PopStyleVar();

        gui.Render(*renderer, camera, scene, vsync);


        window.Update();
        dt = 1.0f / ImGui::GetIO().Framerate; // In seconds
    }

    glDeleteVertexArrays(1, &VAO); 
    glDeleteBuffers(1, &VBO); 
    glDeleteBuffers(1, &IBO); 
    glDeleteBuffers(1, &BVH_UBO); 

    return -1;
}

void SetupQuad(uint32_t& VAO, uint32_t& VBO, uint32_t& IBO)
{
    float vertices[] = 
    {
        // pos                 // col
        -1.0f, -1.0f, 0.0f,    0.25f, 0.52f, 0.96f,
         1.0f, -1.0f, 0.0f,    0.86f, 0.27f, 0.22f,
         1.0f,  1.0f, 0.0f,    0.96f, 0.71f,  0.0f,
        -1.0f,  1.0f, 0.0f,    0.06f, 0.62f, 0.35f
    };

    uint32_t indices[] = 
    {
        0, 1, 2,
        2, 3, 0
    };
 
    glGenVertexArrays  (1, &VAO); 
    glGenBuffers       (1, &VBO); 
    glGenBuffers       (1, &IBO); 
    
    glBindVertexArray  (VAO); 
    glBindBuffer       (GL_ARRAY_BUFFER, VBO); 
    glBufferData       (GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 
    
    glBindBuffer       (GL_ELEMENT_ARRAY_BUFFER, IBO); 
    glBufferData       (GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

    // position attrib
    glVertexAttribPointer      (0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); 
    glEnableVertexAttribArray  (0); 

    // colour attrib  
    glVertexAttribPointer      (1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float))); 
    glEnableVertexAttribArray  (1); 
}

