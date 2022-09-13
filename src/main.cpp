#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "shader.h"
#include "window.h"
#include "framebuffer.h"

int main(void) 
{
    Window window("OpenGL Raytracer", 800, 600);
    FramebufferSpec FBspec = FramebufferSpec();
    FBspec.width = 1000;
    FBspec.height = 1000;
    Framebuffer fb(FBspec);
    // glViewport(0, 0, FBspec.width, FBspec.height);

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

    bool show_demo_window = true;

    float u_SphereCol[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    float prevTime = 0.0;
    float crntTime = 0.0;
    float timeDiff;
    unsigned int counter = 0;
    float FPS, renderTime;

    while (!window.Closed())
    {
        crntTime = glfwGetTime();
        timeDiff = crntTime - prevTime;
        counter++;
        if (timeDiff >= 1.0 / 30.0)
        {
            FPS = (1.0 / timeDiff) * counter;
            renderTime = (timeDiff / counter) * 1000;
            prevTime = crntTime;
            counter = 0;
        }

        // Input
        window.ProcessInput();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        window.Clear();

        // Bind custom framebuffer so frame can be rendered onto texture for ImGui::Image to display onto panel
        fb.Bind();

        shader.Bind();
        shader.SetUniform2f("u_resolution", FBspec.width, FBspec.height);
        shader.SetUniform4f("u_SphereCol", u_SphereCol[0], u_SphereCol[1], u_SphereCol[2], u_SphereCol[3]);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        fb.Unbind();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
        ImGui::Begin("RT Viewport", 0, ImGuiWindowFlags_NoTitleBar);
        ImVec2 MaxSpaceAvail = ImGui::GetContentRegionAvail();
        ImGui::Image((void*)(intptr_t)fb.GetTextureID(), MaxSpaceAvail, {0, 1}, {1, 0});
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Begin("Settings");
        ImGui::Text("Render time: %f ms", renderTime);
        ImGui::Text("FPS: %i", (int)FPS);
        ImGui::ColorEdit4("Sphere Colour", u_SphereCol);
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
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
}
