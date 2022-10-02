#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "shader.h"
#include "window.h"
#include "framebuffer.h"
#include "camera.h"
#include "renderer.h"

int main(void) 
{
    Window window("OpenGL Raytracer", 800, 600);
    // Renderer renderer;
    unsigned int ViewportWidth = 1000;
    unsigned int ViewportHeight = 1000;
    FramebufferSpec FBspec;
    FBspec.width = ViewportWidth;
    FBspec.height = ViewportHeight;
    Framebuffer fb(FBspec);

    Camera camera = Camera(window.GetWindow(), 45.0f, 0.01f, 100.0f);

    float vertices[] = {
        // pos                 // col
        -1.0f, -1.0f, 0.0f,    0.25f, 0.52f, 0.96f,
         1.0f, -1.0f, 0.0f,    0.86f, 0.27f, 0.22f,
         1.0f,  1.0f, 0.0f,    0.96f, 0.71f,  0.0f,
        -1.0f,  1.0f, 0.0f,    0.06f, 0.62f, 0.35f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int       VAO, VBO, IBO;
 
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
    
    Shader shader("res/shaders/vert.vs", "res/shaders/frag.fs");

    // Initialise ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 410");

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 10.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    float u_SphereCol[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float u_LightDirection[3] = { -1.0f, -1.0f, -1.0f };
    float LastFrameTime = 0.0;
    float FrameTime = 0.0;
    float TimeStep = 0.0333;
    int FPS = 0;

    while (!window.Closed())
    {
        // Input
        window.ProcessInput();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        fb.OnResize(ViewportWidth, ViewportHeight);
        camera.OnResize(ViewportWidth, ViewportHeight);
        camera.OnUpdate(TimeStep);
        
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        window.Clear();

        // Bind custom framebuffer so frame can be rendered onto texture for ImGui::Image to display onto panel
        fb.Create();
        fb.Bind();
        glClear(GL_COLOR_BUFFER_BIT);

        shader.Bind();
        shader.SetUniform2f("u_Resolution", ViewportWidth, ViewportHeight);

        shader.SetUniform3f("u_RayOrigin", camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        shader.SetUniform4m("u_InverseProjection", camera.GetInverseProjection());
        shader.SetUniform4m("u_InverseView", camera.GetInverseView());

        shader.SetUniform4f("u_SphereCol", u_SphereCol[0], u_SphereCol[1], u_SphereCol[2], u_SphereCol[3]);

        shader.SetUniform3f("u_LightDirection", u_LightDirection[0], u_LightDirection[1], u_LightDirection[2]);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        shader.Unbind();    
        fb.Unbind();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Game Window", 0, ImGuiWindowFlags_NoTitleBar);

        ViewportWidth = ImGui::GetContentRegionAvail().x;
        ViewportHeight = ImGui::GetContentRegionAvail().y;
        
        ImGui::Image((void*)(intptr_t)fb.GetTextureID(), { (float)ViewportWidth, (float)ViewportHeight }, {0, 1}, {1, 0});

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Begin("Settings");
        ImGui::Text("Render time: %.3f ms", FrameTime * 1000);
        ImGui::Text("FPS: %i", (int)FPS);
        ImGui::ColorEdit3("Sphere Colour", u_SphereCol);
        ImGui::DragFloat3("Light Direction: ", u_LightDirection, 0.01);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        window.Update();
        fb.Destroy();

        float time = glfwGetTime();
        FrameTime = time - LastFrameTime;
        TimeStep = glm::min<float>(FrameTime, 0.0333f);
        LastFrameTime = time;
        FPS = (int) (1 / FrameTime);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
}
