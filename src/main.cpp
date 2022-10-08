#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "shader.h"
#include "window.h"
#include "framebuffer.h"
#include "camera.h"
#include "renderer.h"
#include "scene.h"

void SetupScene(Scene& scene);
void SetupViewportImage(Renderer renderer, Scene scene, uint& VAO, uint& VBO, uint& IBO);

int main(void) 
{
    uint ViewportWidth = 1000, ViewportHeight = 1000;
    Window window("OpenGL Raytracer", 800, 600);
    Shader shader("res/shaders/vert.vs", "res/shaders/frag.fs");
    // std::unique_ptr<Shader> shaderPtr = std::make_unique<Shader>(shader);
    Renderer renderer(shader, ViewportWidth, ViewportHeight);
    Camera camera = Camera(45.0f, 0.01f, 100.0f);
    Scene scene = Scene();

    uint VAO, VBO, IBO;
    
    SetupScene(scene);
    SetupViewportImage(renderer, scene, VAO, VBO, IBO);

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

    float u_LightDirection[3] = { -1.0f, -1.0f, -1.0f };
    float LastFrameTime = 0.0;
    float FrameTime = 0.0;
    float TimeStep = 0.0333;
    uint FPS = 0;

    while (!window.Closed())
    {
        // Input
        window.ProcessInput();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        renderer.OnResize(ViewportWidth, ViewportHeight);
        camera.OnResize(renderer.GetViewportWidth(), renderer.GetViewportHeight());
        camera.OnUpdate(TimeStep, window.GetWindow());

        renderer.SetClearColour(glm::vec4(1.0f));
        renderer.Clear();
        shader.Bind();
        shader.SetUniformVec3("u_LightDirection", u_LightDirection[0], u_LightDirection[1], u_LightDirection[2]);
        renderer.Render(scene, camera, VAO);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoTitleBar);

        ViewportWidth = ImGui::GetContentRegionAvail().x;
        ViewportHeight = ImGui::GetContentRegionAvail().y;
        
        ImGui::Image((void*)(intptr_t)renderer.GetViewportFramebuffer().GetTextureID(), { (float)ViewportWidth, (float)ViewportHeight }, {0, 1}, {1, 0});

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Begin("Settings");
        ImGui::Text("Render time: %.3f ms", FrameTime * 1000);
        ImGui::Text("FPS: %i", (int)FPS);
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
        renderer.GetViewportFramebuffer().Destroy();

        float time = glfwGetTime();
        FrameTime = time - LastFrameTime;
        TimeStep = glm::min<float>(FrameTime, 0.0333f);
        LastFrameTime = time;
        FPS = (uint) (1 / FrameTime);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
}

void SetupScene(Scene& scene)
{
    {
        Sphere sphere;
        sphere.Position = { 0.0f, 0.0f, 0.0f, 1.0f };
        sphere.Albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
        scene.Spheres.push_back(sphere);
    }
    {
        Sphere sphere;
        sphere.Position = { 0.0f, -1000.5f, 0.0f, 1.0f };
        sphere.Radius = 1000.0f;
        sphere.Albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
        scene.Spheres.push_back(sphere);
    }

}

void SetupViewportImage(Renderer renderer, Scene scene, uint& VAO, uint& VBO, uint& IBO)
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

    uint SphereCount = scene.Spheres.size();
    renderer.GetShader().SetUniformInt("u_SphereCount", SphereCount);
    // std::cout << scene.Spheres.data() << std::endl;

    uint SpheresUBO;
    glGenBuffers(1, &SpheresUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, SpheresUBO);
    glBufferData(GL_UNIFORM_BUFFER, SphereCount * sizeof(Sphere), scene.Spheres.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    uint block = glGetUniformBlockIndex(renderer.GetShader().GetID(), "SpheresBlock");
    uint bind = 0;
    glUniformBlockBinding(renderer.GetShader().GetID(), block, bind);
}
