#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "consoleLogger.h"
#include "shader.h"
#include "framebuffer.h"
#include "camera.h"
#include "renderer.h"
#include "scene.h"
#include "gui.h"

void SetupUBOs(Renderer& renderer, Scene& scene, uint32_t& ObjectsDataUBO);
void SetupQuad(uint32_t& VAO, uint32_t& VBO, uint32_t& IBO);

int main(void) 
{
    const char* Title = "Path Tracing";
    uint32_t ViewportHeight = 600;
    uint32_t ViewportWidth = ViewportHeight * 16/9;
    Window window = Window(Title, ViewportWidth, ViewportHeight);
    Shader PathTracerShader = Shader("src/shaders/vert.glsl", "src/shaders/path_tracer.glsl");
    Shader AccumShader = Shader("src/shaders/vert.glsl", "src/shaders/accumulation.glsl");
    Shader FinalOutputShader = Shader("src/shaders/vert.glsl", "src/shaders/final_output.glsl");
    Renderer renderer = Renderer(PathTracerShader, AccumShader, FinalOutputShader, ViewportWidth, ViewportHeight);
    Gui gui = Gui(window);
    Camera camera = Camera({0.0f, 2.0f, 6.0f}, 90.0f, 0.01f, 100.0f);
    Scene scene = Scene();

    uint32_t VAO, VBO, IBO, ObjectsDataUBO;
    
    scene.CustomScene();
    SetupQuad(VAO, VBO, IBO);
    SetupUBOs(renderer, scene, ObjectsDataUBO);

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
            glBindBuffer(GL_UNIFORM_BUFFER, ObjectsDataUBO); GLCall;
            int offset = 0;
            int ObjCount = scene.spheres.size();
            // Set Sphere object data
            glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &ObjCount);
            offset += sizeof(glm::vec4);
            glBufferSubData(GL_UNIFORM_BUFFER, offset, ObjCount * sizeof(Sphere), scene.spheres.data());
        }

        renderer.Render(scene, camera, VAO);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoTitleBar);

        ViewportWidth = ImGui::GetContentRegionAvail().x;
        ViewportHeight = ImGui::GetContentRegionAvail().y;

        uint32_t image = renderer.GetViewportFramebuffer().GetTextureID();

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
    glDeleteBuffers(1, &ObjectsDataUBO); GLCall;
}

void SetupUBOs(Renderer& renderer, Scene& scene, uint32_t& ObjectsDataUBO)
{
    renderer.GetShader()->Bind();
    int ObjCount = scene.spheres.size();
    
    uint32_t block = glGetUniformBlockIndex(renderer.GetShader()->GetID(), "ObjectData"); GLCall;
    uint32_t bind = 0;
    glUniformBlockBinding(renderer.GetShader()->GetID(), block, bind); GLCall;

    glGenBuffers(1, &ObjectsDataUBO); GLCall;
    glBindBuffer(GL_UNIFORM_BUFFER, ObjectsDataUBO); GLCall;
    glBindBufferBase(GL_UNIFORM_BUFFER, bind, ObjectsDataUBO); GLCall;
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) + ObjCount * sizeof(Sphere), nullptr, GL_STATIC_DRAW); GLCall;

    {
        int offset = 0;

        // Set Sphere object data
        glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &ObjCount);
        offset += sizeof(glm::vec4);

        glBufferSubData(GL_UNIFORM_BUFFER, offset, ObjCount * sizeof(Sphere), scene.spheres.data());
    }

    renderer.GetShader()->Unbind();
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
