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

void SetupScene(Scene& scene);
void SetupViewportImage(const Renderer& renderer, const Scene& scene, uint& VAO, uint& VBO, uint& IBO, uint& SpheresUBO);

int main(void) 
{
    uint ViewportWidth = 1000, ViewportHeight = 1000;
    Window window = Window("GLRT", 800, 600);
    Shader shader = Shader("res/shaders/vert.glsl", "res/shaders/frag.glsl");
    Renderer renderer = Renderer(shader, ViewportWidth, ViewportHeight);
    Camera camera = Camera(60.0f, 0.01f, 100.0f);
    Scene scene = Scene();

    uint VAO, VBO, IBO, SpheresUBO;
    
    SetupScene(scene);
    SetupViewportImage(renderer, scene, VAO, VBO, IBO, SpheresUBO);

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

    float LastFrameTime = 0.0;
    float FrameTime = 0.0;
    float dt = 0.0333;
    uint FPS = 0;
    bool vsync = true;

    while (!window.Closed())
    {
        // Input
        window.ProcessInput();
        if (vsync) { glfwSwapInterval(1); }
        else { glfwSwapInterval(0); }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        renderer.OnResize(ViewportWidth, ViewportHeight);
        camera.OnResize(renderer.GetViewportWidth(), renderer.GetViewportHeight());
        camera.OnUpdate(dt, window.GetWindow());

        //glBindBuffer(GL_UNIFORM_BUFFER, SpheresUBO); GLCall;
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), scene.Spheres.size() * sizeof(Sphere), scene.Spheres.data()); GLCall;
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

        ImGui::Begin("Analytics");
        ImGui::Checkbox("V-Sync", &vsync);
        ImGui::Text("Render time: %.3f ms", FrameTime * 1000);
        ImGui::Text("FPS: %i", (int)FPS);
        ImGui::BulletText("Press 1/2 to disable/enable mouse rotation");
        ImGui::End();
        
        ImGui::Begin("Scene");
        float u_LightDirection[3] = {scene.lightDirection.x, scene.lightDirection.y, scene.lightDirection.z};
        ImGui::Text("[Camera Position]");
        ImGui::Text("< %.2f, %.2f, %.2f >", camera.GetPosition().x, camera.GetPosition().y , camera.GetPosition().z);
        ImGui::Separator();
        ImGui::Text("[Light Direction]");
        ImGui::DragFloat3("", u_LightDirection, 0.01);
        scene.lightDirection = glm::vec3(u_LightDirection[0], u_LightDirection[1], u_LightDirection[2]);

        for (int i = 0; i < scene.Spheres.size(); i++)
        {
            ImGui::PushID(i);

            Sphere& sphere = scene.Spheres[i];
            ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
            ImGui::DragFloat("Radius", &sphere.Position.w, 0.1f, 0.1f);
            ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.Mat.Albedo));

            ImGui::Separator();
            ImGui::PopID();
        }

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
        dt = glm::min<float>(FrameTime, 0.0333f);
        LastFrameTime = time;
        FPS = (uint) (1 / FrameTime);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO); GLCall;
    glDeleteBuffers(1, &VBO); GLCall;
    glDeleteBuffers(1, &IBO); GLCall;
}

void SetupScene(Scene& scene)
{
    {
        Sphere sphere;
        scene.Spheres.push_back(sphere);
    }
    {
        Sphere sphere;
        sphere.Position = { 5.0f, 0.0f, 0.0f, 1.0f };
        scene.Spheres.push_back(sphere);
    }
    {
        Sphere sphere;
        sphere.Position = glm::vec4(0.0f, -101.0f, 0.0f, 100.0f);
        //sphere.Mat.Albedo = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);
        scene.Spheres.push_back(sphere);
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
    int SphereCount = scene.Spheres.size();
    
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

        glBufferSubData(GL_UNIFORM_BUFFER, offset, SphereCount * sizeof(Sphere), scene.Spheres.data());
    }

    renderer.GetShader()->Unbind();
}
