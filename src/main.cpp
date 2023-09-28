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
    uint32_t ViewportHeight = 600;
    uint32_t ViewportWidth = ViewportHeight * 16/9;
    Window window = Window(Title, ViewportWidth, ViewportHeight);
    Gui gui = Gui(window);
    Scene scene = Scene();
    Camera camera = Camera({0.0f, 2.0f, 6.0f}, 90.0f, 0.01f, 100.0f);
    Renderer renderer = Renderer(ViewportWidth, ViewportHeight, &scene, &camera);

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
    glGenVertexArrays(1, &debug_vao); GLCall;
    glBindVertexArray(debug_vao); GLCall;

    uint32_t vbo_vertices;
    glGenBuffers(1, &vbo_vertices); GLCall;
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices); GLCall;
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); GLCall;

    uint32_t ibo_elements;
    glGenBuffers(1, &ibo_elements); GLCall;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements); GLCall;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW); GLCall;

    glEnableVertexAttribArray(0); GLCall;
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); GLCall;

    renderer.debugVAO = debug_vao;

    SetupQuad(VAO, VBO, IBO);

    float LastFrame = 0.0f;
    float dt = 0.0f;
    bool vsync = false;

    while (!window.Closed())
    {
        float CurrentFrame = glfwGetTime();

        // Input
        window.ProcessInput();
        vsync == true ? glfwSwapInterval(1) : glfwSwapInterval(0);

        gui.NewFrame();

        renderer.OnResize(ViewportWidth, ViewportHeight);
        camera.OnResize(renderer.GetViewportWidth(), renderer.GetViewportHeight());

        if (camera.OnUpdate(dt, &window)) 
            renderer.ResetSamples();
        
        if (!renderer.b_Pause)
        {
            scene.Data.SunDirection = scene.lightDirection;
            scene.Data.Depth = scene.maxRayDepth;
            scene.Data.SelectedPrimIdx = scene.SphereIdx;
            scene.Data.Day = scene.day;

            renderer.Render(scene, camera, VAO);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoTitleBar);

        ViewportWidth = ImGui::GetContentRegionAvail().x;
        ViewportHeight = ImGui::GetContentRegionAvail().y;

        uint32_t image = renderer.GetViewportFramebuffer().GetTextureID();
        ImGui::Image((void*)(intptr_t)image, {(float)ViewportWidth, (float)ViewportHeight}, {0, 1}, {1, 0});

        ImGui::End();
        ImGui::PopStyleVar();

        gui.Render(renderer, camera, scene, vsync);


        window.Update();
        dt = CurrentFrame - LastFrame;
        LastFrame = CurrentFrame;
    }

    glDeleteVertexArrays(1, &VAO); GLCall;
    glDeleteBuffers(1, &VBO); GLCall;
    glDeleteBuffers(1, &IBO); GLCall;
    glDeleteBuffers(1, &BVH_UBO); GLCall;
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
}

