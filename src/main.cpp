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
void CreateAndBindBuffer(const Renderer& renderer, const char* name, uint32_t& buffer, uint32_t bind);
void UpdateObjectsUBO(const Scene& scene);
void DrawBbox(Shader& shader, BVH_Node node, uint32_t vao);
void DrawTree(Shader& shader, BVH_Node* node, uint32_t vao);

const uint32_t MAX_SPHERES = 500;

int main(void) 
{
    const char* Title = "Path Tracing";
    uint32_t ViewportHeight = 600;
    uint32_t ViewportWidth = ViewportHeight * 16/9;
    Window window = Window(Title, ViewportWidth, ViewportHeight);
    Shader PathTracerShader = Shader("src/shaders/vert.glsl", "src/shaders/pathTracer.glsl");
    Shader AccumShader = Shader("src/shaders/vert.glsl", "src/shaders/accumulation.glsl");
    Shader FinalOutputShader = Shader("src/shaders/vert.glsl", "src/shaders/postProcessing.glsl");
    Shader DebugBVH = Shader("src/shaders/debugVert.glsl", "src/shaders/debug.glsl");
    Renderer renderer = Renderer(PathTracerShader, AccumShader, FinalOutputShader, ViewportWidth, ViewportHeight);
    Gui gui = Gui(window);
    Camera camera = Camera({0.0f, 2.0f, 6.0f}, 90.0f, 0.01f, 100.0f);
    Scene scene = Scene();

    uint32_t VAO, VBO, IBO, ObjectsDataUBO, BVH_UBO;

    // Pre-render setup, e.g. Load scene, Build BVH...
    scene.RTIW();
    BVH bvh = BVH(scene.spheres);
    int treeSize = bvh.CountNodes(bvh.bvh_root);
    renderer.BVH = &bvh;

    renderer.GetShader()->Bind(); GLCall;

    CreateAndBindBuffer(renderer, "ObjectData", ObjectsDataUBO, 0);
    int mem = 2 * sizeof(glm::vec4) + MAX_SPHERES * sizeof(GPUSphere) + 50 * sizeof(GPUAABB);
    glBufferData(GL_UNIFORM_BUFFER, mem, nullptr, GL_DYNAMIC_DRAW); GLCall;
    UpdateObjectsUBO(scene);

    CreateAndBindBuffer(renderer, "BVH", BVH_UBO, 1);
    glBufferData(GL_UNIFORM_BUFFER, treeSize * sizeof(LinearBVH_Node), bvh.flat_root, GL_STATIC_DRAW); GLCall;

    renderer.GetShader()->Unbind(); GLCall;

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

        bool cameraIsMoving;
        if (camera.type == 0) 
            cameraIsMoving = camera.FPS(dt, &window);

        if (camera.type == 1) 
            cameraIsMoving = camera.Cinematic(dt, &window);

        if (cameraIsMoving) 
            renderer.ResetSamples();
        
        if (!renderer.b_Pause)
        {
            // Update UBOs
            glBindBuffer(GL_UNIFORM_BUFFER, ObjectsDataUBO); GLCall;
            UpdateObjectsUBO(scene);

            renderer.Render(scene, camera, VAO);

            if (vsync)
            {
                renderer.GetViewportFramebuffer().Bind(); 
                glViewport(0, 0, ViewportWidth, ViewportHeight);
                
                DebugBVH.Bind();

                DebugBVH.SetUniformMat4("u_View", camera.GetView());
                DebugBVH.SetUniformMat4("u_Projection", camera.GetProjection());
                // DebugBVH.SetUniformVec2("u_Resolution", (float)ViewportWidth, (float)ViewportHeight);

                // std::cout << "\nRoot of Tree" << std::endl;
                DrawTree(DebugBVH, bvh.bvh_root, debug_vao);
                DebugBVH.Unbind();

                renderer.GetViewportFramebuffer().Unbind();  
            }
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

        renderer.GetShader()->Bind(); GLCall;
        if (renderer.GetShader()->b_Reloaded)
        {
            renderer.BVH->b_Rebuilt = true;
            std::cout << "Shader Reloaded" << std::endl;
            int mem = 2 * sizeof(glm::vec4) + MAX_SPHERES * sizeof(GPUSphere) + 50 * sizeof(GPUAABB);
            glBufferData(GL_UNIFORM_BUFFER, mem, nullptr, GL_DYNAMIC_DRAW); GLCall;
            UpdateObjectsUBO(scene);
            renderer.GetShader()->b_Reloaded = false;
        }

        if (renderer.BVH->b_Rebuilt)
        {
            treeSize = renderer.BVH->CountNodes(renderer.BVH->bvh_root);
            // std::cout << treeSize << std::endl;
            glBindBuffer(GL_UNIFORM_BUFFER, BVH_UBO); GLCall;
            glBufferData(GL_UNIFORM_BUFFER, treeSize * sizeof(LinearBVH_Node), renderer.BVH->flat_root, GL_STATIC_DRAW); GLCall;
            renderer.BVH->b_Rebuilt = false;
        }
        renderer.GetShader()->Unbind(); GLCall;

        window.Update();
        dt = CurrentFrame - LastFrame;
        LastFrame = CurrentFrame;
    }

    glDeleteVertexArrays(1, &VAO); GLCall;
    glDeleteBuffers(1, &VBO); GLCall;
    glDeleteBuffers(1, &IBO); GLCall;
    glDeleteBuffers(1, &ObjectsDataUBO); GLCall;
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

void CreateAndBindBuffer(const Renderer& renderer, const char* name, uint32_t& buffer, uint32_t bind)
{
    uint32_t block = glGetUniformBlockIndex(renderer.GetShader()->GetID(), name); GLCall;
    glUniformBlockBinding(renderer.GetShader()->GetID(), block, bind); GLCall;

    glGenBuffers(1, &buffer); GLCall;
    glBindBuffer(GL_UNIFORM_BUFFER, buffer); GLCall;
    glBindBufferBase(GL_UNIFORM_BUFFER, bind, buffer); GLCall;
}

void UpdateObjectsUBO(const Scene& scene)
{
    int offset = 0;
    int n_Spheres = scene.spheres.size();
    int n_AABBs = scene.aabbs.size();
    // int n_Lights = 0; // scene.lights.size();

    std::vector<GPUSphere> spheres;
    for (int i = 0; i < n_Spheres; i++)
    {
        spheres.push_back(scene.spheres[i].sphere);
    }

    // Set Sphere object data
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &n_Spheres); GLCall;
    offset += sizeof(glm::vec4);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, n_Spheres * sizeof(GPUSphere), spheres.data()); GLCall;
    offset += (MAX_SPHERES) * sizeof(GPUSphere);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &n_AABBs); GLCall;
    offset += sizeof(glm::vec4);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, n_AABBs * sizeof(GPUAABB), scene.aabbs.data()); GLCall;
    offset += (50) * sizeof(GPUAABB);
    // glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &n_Lights); GLCall;
    // offset += sizeof(glm::vec4);
    // glBufferSubData(GL_UNIFORM_BUFFER, offset, scene.lights.size() * sizeof(Light), scene.lights.data()); GLCall;
}

void DrawBbox(Shader& shader, BVH_Node node, uint32_t vao)
{
    glm::vec3 scale = node.bbox.bMax - node.bbox.bMin;
    glm::vec3 center = glm::vec3(
        (node.bbox.bMin.x + node.bbox.bMax.x) / 2, 
        (node.bbox.bMin.y + node.bbox.bMax.y) / 2, 
        (node.bbox.bMin.z + node.bbox.bMax.z) / 2);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), scale);

    // std::cout << "dim: vec3(" 
    //     << scale.x << ", " 
    //     << scale.y << ", " 
    //     << scale.z << ")" << std::endl;
    // std::cout << "pos: vec3("
    //     << center.x << ", " 
    //     << center.y << ", " 
    //     << center.z << ")" << std::endl;
    // std::cout << std::endl;

    shader.SetUniformMat4("u_Model", model);

    glBindVertexArray(vao); GLCall;
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0); GLCall;
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (GLvoid*)(4 * sizeof(uint32_t))); GLCall;
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, (GLvoid*)(8 * sizeof(uint32_t))); GLCall;
    glBindVertexArray(0); GLCall;
}

void DrawTree(Shader& shader, BVH_Node* node, uint32_t vao)
{
    if (node == nullptr)
        return;
    
    // std::cout << "Calling DrawBbox" << std::endl;
    DrawBbox(shader, *node, vao);

    DrawTree(shader, node->left, vao);
    DrawTree(shader, node->right, vao);
}


