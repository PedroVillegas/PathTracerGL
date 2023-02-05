#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "consoleLogger.h"
#include "shader.h"
#include "window.h"
#include "framebuffer.h"
#include "camera.h"
#include "renderer.h"
#include "scene.h"

#include <glm/glm.hpp>

void InitImGui(GLFWwindow* window);
void SetupStyle();
void SetupScene(Scene& scene);
void SetupViewportImage(const Renderer& renderer, const Scene& scene, uint& VAO, uint& VBO, uint& IBO, uint& SpheresUBO);

int main(void) 
{
    uint ViewportWidth = 1000, ViewportHeight = 1000;
    Window window = Window("Path Tracing", 1000, 600);
    Shader PathTracerShader = Shader("res/shaders/vert.glsl", "res/shaders/path_tracer.glsl");
    Shader AccumShader = Shader("res/shaders/vert.glsl", "res/shaders/accumulation.glsl");
    Shader FinalOutputShader = Shader("res/shaders/vert.glsl", "res/shaders/final_output.glsl");
    Renderer renderer = Renderer(PathTracerShader, AccumShader, FinalOutputShader, ViewportWidth, ViewportHeight);
    Camera camera = Camera({0.0f, 0.0f, 3.0f}, 90.0f, 0.01f, 100.0f);
    Scene scene = Scene();

    uint VAO, VBO, IBO, SpheresUBO;
    
    SetupScene(scene);
    SetupViewportImage(renderer, scene, VAO, VBO, IBO, SpheresUBO);
    InitImGui(window.GetWindow());
    SetupStyle();

    float dt = 0.0333;
    bool vsync = true;

    while (!window.Closed())
    {
        // Input
        window.ProcessInput();
        vsync == true ? glfwSwapInterval(1) : glfwSwapInterval(0);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        renderer.OnResize(ViewportWidth, ViewportHeight);
        camera.OnResize(renderer.GetViewportWidth(), renderer.GetViewportHeight());
        bool cameraIsMoving = camera.OnUpdate(dt, window.GetWindow());
        // camera.CinematicMovement(dt, window.GetWindow());

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
        {
            // std::cout << "Successfully obtained framebuffer texture ID." << std::endl;
            ImGui::Image((void*)(intptr_t)image, { (float)ViewportWidth, (float)ViewportHeight }, {0, 1}, {1, 0});
        }

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Begin("Debug");
        ImGui::Text("Viewport: %i x %i", ViewportWidth, ViewportHeight);
        ImGui::Text("[1/2] to toggle movement");
        ImGui::Checkbox("V-Sync", &vsync);
        ImGui::Text("Render time %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
        
        ImGui::Begin("Scene");
        ImGui::Text("Iterations: %i", renderer.GetIterations());
        if (ImGui::Button("Reset Samples")) renderer.ResetSamples();
        if (ImGui::SliderInt("SPP", &scene.samplesPerPixel, 1, 50)) renderer.ResetSamples();
        if (ImGui::SliderInt("Max Ray Depth", &scene.maxRayDepth, 1, 50)) renderer.ResetSamples();
        ImGui::Separator();
        ImGui::Text("Camera Position : %.2f %.2f %.2f ", camera.GetPosition().x, camera.GetPosition().y , camera.GetPosition().z);
        ImGui::Text("Camera Momentum : %.2f %.2f %.2f", camera.GetMomentum().x, camera.GetMomentum().y , camera.GetMomentum().z);
        ImGui::Text("Camera Direction: %.2f %.2f %.2f", camera.GetDirection().x, camera.GetDirection().y , camera.GetDirection().z);
        if (ImGui::SliderInt("FOV", &camera.horizontalFOV, 60, 120))
        {
            camera.SetFov(camera.horizontalFOV);
            camera.RecalculateProjection();
            renderer.ResetSamples();
        }
        
        ImGui::Separator();

        for (int i = 0; i < scene.spheres.size(); i++)
        {
            ImGui::PushID(i);

            Sphere& s = scene.spheres[i];
            if (ImGui::ColorEdit3("Albedo", glm::value_ptr(s.mat.albedo))) renderer.ResetSamples();
            if (ImGui::DragFloat3("Position", glm::value_ptr(s.position), 0.1f)) renderer.ResetSamples();
            if (ImGui::DragFloat("Radius", &s.position.w, 0.05f, 0.1f, 1000.0f)) renderer.ResetSamples();
            if (ImGui::DragFloat("Roughness", &s.mat.roughness, 0.002f, 0.0f, 1.0f)) renderer.ResetSamples();

            ImGui::Separator();
            ImGui::PopID();
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        window.Update();
        dt = glm::min<float>(1000.0f / ImGui::GetIO().Framerate, 0.0333f);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO); GLCall;
    glDeleteBuffers(1, &VBO); GLCall;
    glDeleteBuffers(1, &IBO); GLCall;
    glDeleteBuffers(1, &SpheresUBO); GLCall;
}

void SetupScene(Scene& scene)
{
    {
        Sphere centre_sphere;
        centre_sphere.mat.albedo = { 0.7f, 0.3f, 0.3f, 1.0f };
        scene.spheres.push_back(centre_sphere);
    }
    {
        Sphere left_sphere;
        left_sphere.mat.albedo = { 0.8f, 0.8f, 0.8f, 1.0f };
        left_sphere.position = { -2.0f, 0.0f, 0.0f, 1.0f };
        left_sphere.mat.roughness = 0.0f;
        scene.spheres.push_back(left_sphere);
    }
    {
        Sphere right_sphere;
        right_sphere.mat.albedo = { 0.1f, 0.2f, 0.5f, 1.0f };
        right_sphere.position = { 2.0f, 0.0f, 0.0f, 1.0f };
        right_sphere.mat.roughness = 0.0f;
        scene.spheres.push_back(right_sphere);
    }
    {
        Sphere ground_sphere;
        ground_sphere.mat.albedo = { 0.8f, 0.8f, 0.0f, 1.0f };
        ground_sphere.position = { 0.0f, -301.0f, 0.0f, 300.0f };
        scene.spheres.push_back(ground_sphere);
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

void InitImGui(GLFWwindow* window)
{
    // Initialise ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

void SetupStyle()
{
    // https://github.com/ocornut/imgui/issues/707#issuecomment-252413954

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.3f;
    style.FrameRounding = 2.3f;
    style.ScrollbarRounding = 0;

    style.Colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.00f, 0.00f, 0.01f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.1f, 0.1f, 0.1f, 0.99f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
    style.Colors[ImGuiCol_Separator]             = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

    /*
    ImGui::GetStyle().FrameRounding = 4.0f;
    ImGui::GetStyle().GrabRounding = 4.0f;
    
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 0.37f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 0.16f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 0.57f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.21f, 0.27f, 0.31f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.55f, 0.73f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.10f, 0.15f, 0.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.41f, 0.78f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.25f, 0.29f, 0.80f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
    colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
    colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    */

    /*
    ImGuiStyle& style = ImGui::GetStyle();
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 10.0f;
        style.Colors[ImGuiCol_WindowBg].w = 0.2f;
    }
    */

    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    //style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 5.0f;
    style.GrabRounding = 3.0f;
}
