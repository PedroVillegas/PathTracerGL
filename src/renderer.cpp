#include "imgui/imgui.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stdio.h>

#include "consoleLogger.h"
#include "renderer.h"

void DrawBbox(Shader& shader, BVH_Node node, uint32_t vao);
void DrawTree(Shader& shader, BVH_Node* node, uint32_t vao);

Renderer::Renderer(
    uint32_t ViewportWidth,
    uint32_t ViewportHeight,
    Scene* scene,
    Camera* camera)
    :
    m_Camera(camera),
    m_Scene(scene),
    m_ViewportWidth(ViewportWidth),
    m_ViewportHeight(ViewportHeight)
{
    m_ViewportSpec.width = m_ViewportWidth;
    m_ViewportSpec.height = m_ViewportHeight;
    m_PathTraceFBO = Framebuffer(m_ViewportSpec);
    m_PathTraceFBO.Create();
    m_AccumulationFBO = Framebuffer(m_ViewportSpec);
    m_AccumulationFBO.Create();
    m_FinalOutputFBO = Framebuffer(m_ViewportSpec);
    m_FinalOutputFBO.Create();

    m_PathTraceShader = new Shader("src/shaders/vert.glsl", "src/shaders/pathTracer.glsl");
    m_AccumShader = new Shader("src/shaders/vert.glsl", "src/shaders/accumulation.glsl");
    m_FinalOutputShader = new Shader("src/shaders/vert.glsl", "src/shaders/postProcessing.glsl");
    m_BVHDebugShader = new Shader("src/shaders/debugVert.glsl", "src/shaders/debug.glsl");

    m_Scene->TestPrims();
    m_BVH = new BVH(m_Scene->spheres);

    // Setup BVH UBO
    int treeSize = m_BVH->CountNodes(m_BVH->bvh_root);
    glGenBuffers(1, &m_BVHBlockBuffer); GLCall;
    glBindBuffer(GL_UNIFORM_BUFFER, m_BVHBlockBuffer); GLCall;
    glBufferData(GL_UNIFORM_BUFFER, treeSize * sizeof(LinearBVH_Node), m_BVH->flat_root, GL_STATIC_DRAW); GLCall;
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_BVHBlockBuffer); GLCall;
    m_PathTraceShader->SetUBO("BVH", 0);

    // Setup PrimsBlock UBO
    glGenBuffers(1, &m_PrimsBlockBuffer); GLCall;
    glBindBuffer(GL_UNIFORM_BUFFER, m_PrimsBlockBuffer); GLCall;
    int mem = sizeof(glm::vec4) + MAX_SPHERES * sizeof(GPUSphere)
            + MAX_AABBS * sizeof(GPUAABB) + MAX_LIGHTS * sizeof(Light)
            + 100 * sizeof(Primitive);
    glBufferData(GL_UNIFORM_BUFFER, mem, nullptr, GL_DYNAMIC_DRAW); GLCall;
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_PrimsBlockBuffer); GLCall;
    m_PathTraceShader->SetUBO("PrimsBlock", 1);

    // Setup SceneBlock UBO
    glGenBuffers(1, &m_SceneBlockBuffer); GLCall;
    glBindBuffer(GL_UNIFORM_BUFFER, m_SceneBlockBuffer); GLCall;
    glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneBlock), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_SceneBlockBuffer); GLCall;
    m_PathTraceShader->SetUBO("SceneBlock", 2);

    // Setup CameraBlock UBO
    glGenBuffers(1, &m_CameraBlockBuffer); GLCall;
    glBindBuffer(GL_UNIFORM_BUFFER, m_CameraBlockBuffer); GLCall;
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_CameraBlockBuffer); GLCall;
    m_PathTraceShader->SetUBO("CameraBlock", 3);
}

Renderer::~Renderer()
{
    m_PathTraceFBO.Destroy();
    m_AccumulationFBO.Destroy();
    m_FinalOutputFBO.Destroy();

    delete m_BVH;
    delete m_BVHDebugShader;
    delete m_PathTraceShader;
    delete m_AccumShader;
    delete m_FinalOutputShader;
}

void Renderer::UpdateBuffers()
{
    if (m_PathTraceShader->b_Reloaded)
    {
        std::cout << "Rebuilding BVH..." << std::endl;
        m_BVH->RebuildBVH(m_Scene->spheres);
        std::cout << "BVH Successfully Rebuilt" << std::endl;
        m_BVH->b_Rebuilt = true;
        int mem = sizeof(glm::vec4) + MAX_SPHERES * sizeof(GPUSphere) + MAX_AABBS * sizeof(GPUAABB) + MAX_LIGHTS * sizeof(Light);
        glBindBuffer(GL_UNIFORM_BUFFER, m_PrimsBlockBuffer);
        glBufferData(GL_UNIFORM_BUFFER, mem, nullptr, GL_DYNAMIC_DRAW); GLCall;
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        m_PathTraceShader->b_Reloaded = false;
    }


    // Update BVH Block only if rebuilt
    if (m_BVH->b_Rebuilt)
    {
        int treeSize = m_BVH->CountNodes(m_BVH->bvh_root);
        // std::cout << treeSize << std::endl;
        glBindBuffer(GL_UNIFORM_BUFFER, m_BVHBlockBuffer); GLCall;
        glBufferData(GL_UNIFORM_BUFFER, treeSize * sizeof(LinearBVH_Node), m_BVH->flat_root, GL_STATIC_DRAW); GLCall;
        glBindBuffer(GL_UNIFORM_BUFFER, 0); GLCall;
        m_BVH->b_Rebuilt = false;
    }

    // Update Scene Block
    glBindBuffer(GL_UNIFORM_BUFFER, m_SceneBlockBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SceneBlock), &m_Scene->Data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Update Camera Block
    glBindBuffer(GL_UNIFORM_BUFFER, m_CameraBlockBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraBlock), &m_Camera->params);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Update Prims Block
    glBindBuffer(GL_UNIFORM_BUFFER, m_PrimsBlockBuffer);
    int offset = 0;
    int n_Spheres = m_Scene->spheres.size();
    int n_AABBs = m_Scene->aabbs.size();
    int n_Lights = m_Scene->lights.size();
    int n_Primitives = m_Scene->primitives.size();
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &n_Spheres); GLCall;
    offset += sizeof(int);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &n_AABBs); GLCall;
    offset += sizeof(int);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &n_Lights); GLCall;
    offset += sizeof(int);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &n_Primitives); GLCall;
    offset += sizeof(int);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, n_Spheres * sizeof(GPUSphere), m_Scene->spheres.data()); GLCall;
    offset += (MAX_SPHERES) * sizeof(GPUSphere);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, n_AABBs * sizeof(GPUAABB), m_Scene->aabbs.data()); GLCall;
    offset += (MAX_AABBS) * sizeof(GPUAABB);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, n_Lights * sizeof(Light), m_Scene->lights.data()); GLCall;
    offset += (MAX_AABBS) * sizeof(Light);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, n_Primitives * sizeof(Primitive), m_Scene->primitives.data()); GLCall;
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::Render(const Scene& scene, const Camera& camera, uint32_t VAO)
{
    SetClearColour(1.0f, 0.0f, 1.0f, 1.0f); GLCall;
    // m_Scene = &scene;
    // m_Camera = &camera;

    // First pass:
    // Render current frame to m_PathTraceFBO using m_AccumulationFBO's texture to continue accumulating samples
    // For first frame the texture will be empty and will not affect the output
    glActiveTexture(GL_TEXTURE0); GLCall;
    glBindTexture(GL_TEXTURE_2D, m_AccumulationFBO.GetTextureID()); GLCall;

    m_PathTraceShader->Bind(); GLCall;
    m_PathTraceShader->SetUniformInt("u_AccumulationTexture", 0); GLCall;
    m_PathTraceShader->SetUniformInt("u_SampleIterations", m_SampleIterations); GLCall;
    m_PathTraceShader->SetUniformInt("u_SamplesPerPixel", m_Scene->samplesPerPixel); GLCall;
    m_PathTraceShader->SetUniformVec2("u_Resolution", float(m_ViewportWidth), float(m_ViewportHeight)); GLCall;

    UpdateBuffers();

    m_PathTraceFBO.Bind(); GLCall;

    Clear(); GLCall;
    glBindVertexArray(VAO); GLCall;
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GLCall;
    glBindVertexArray(0); GLCall;

    m_PathTraceFBO.Unbind(); GLCall;
    m_PathTraceShader->Unbind();

    // Second Pass:
    // This pass is used to copy the previous pass' output (m_PathTraceFBO) onto m_AccumulationFBO which will hold the data 
    // until used again for the first pass of the next frame
    m_AccumShader->Bind(); GLCall;
    m_AccumulationFBO.Bind(); GLCall;

    glActiveTexture(GL_TEXTURE0); GLCall;
    glBindTexture(GL_TEXTURE_2D, m_PathTraceFBO.GetTextureID()); GLCall;

    m_AccumShader->SetUniformInt("u_PathTraceTexture", 0); GLCall;
    m_AccumShader->SetUniformVec2("u_Resolution", float(m_ViewportWidth), float(m_ViewportHeight)); GLCall;

    Clear(); GLCall;
    glBindVertexArray(VAO); GLCall;
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GLCall;
    glBindVertexArray(0); GLCall;

    glActiveTexture(GL_TEXTURE0); GLCall;
    glBindTexture(GL_TEXTURE_2D, m_AccumulationFBO.GetTextureID()); GLCall;

    m_AccumulationFBO.Unbind(); GLCall;
    m_AccumShader->Unbind();

    // Final pass:
    // Now use the texture from either of the previously used FBO and divide by the frame count
    m_FinalOutputShader->Bind(); GLCall;
    m_FinalOutputFBO.Bind(); GLCall;
    m_FinalOutputShader->SetUniformInt("u_PT_Texture", 0); GLCall;
    m_FinalOutputShader->SetUniformVec2("u_Resolution", float(m_ViewportWidth), float(m_ViewportHeight)); GLCall;

    Clear(); GLCall;
    glBindVertexArray(VAO); GLCall;
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GLCall;
    glBindVertexArray(0); GLCall;

    m_FinalOutputFBO.Unbind(); GLCall;
    m_FinalOutputShader->Unbind();
    
    if (b_DrawBVH)
    {
        m_FinalOutputFBO.Bind(); GLCall;
        glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
        m_BVHDebugShader->Bind();

        m_BVHDebugShader->SetUniformMat4("u_View", camera.GetView());
        m_BVHDebugShader->SetUniformMat4("u_Projection", camera.GetProjection());
        // m_BVHDebugShader->SetUniformVec2("u_Resolution", (float)ViewportWidth, (float)ViewportHeight);

        // std::cout << "\nRoot of Tree" << std::endl;
        DrawTree(*m_BVHDebugShader, m_BVH->bvh_root, debugVAO);
        m_BVHDebugShader->Unbind();
        m_FinalOutputFBO.Unbind(); GLCall;
    }

    m_SampleIterations++;
}

void Renderer::SetViewportWidth(uint32_t width)
{
    m_ViewportWidth = width;
}

void Renderer::SetViewportHeight(uint32_t height)
{
    m_ViewportWidth = height;
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
    if (width == m_ViewportWidth && height == m_ViewportHeight)
        return;

    m_ViewportWidth = width;
    m_ViewportHeight = height;

    m_AccumulationFBO.OnResize(m_ViewportWidth, m_ViewportHeight);
    m_PathTraceFBO.OnResize(m_ViewportWidth, m_ViewportHeight);
    m_FinalOutputFBO.OnResize(m_ViewportWidth, m_ViewportHeight);
    b_Pause = false;
    ResetSamples();
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