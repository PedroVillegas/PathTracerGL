#include "imgui/imgui.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stdio.h>

#include "renderer.h"

void DrawBbox(Shader& shader, BVH_Node node, uint32_t vao);
void DrawTree(Shader& shader, BVH_Node* node, uint32_t vao, int currentDepth, int terminationDepth);

Renderer::Renderer(
    uint32_t ViewportWidth,
    uint32_t ViewportHeight,
    Scene* scene)
    :
    m_ViewportWidth(ViewportWidth),
    m_ViewportHeight(ViewportHeight),
    m_Scene(scene)
{
    m_ViewportSpec.width = m_ViewportWidth;
    m_ViewportSpec.height = m_ViewportHeight;
    m_PathTraceFBO = Framebuffer(m_ViewportSpec);
    m_PathTraceFBO.Create();
    m_AccumulationFBO = Framebuffer(m_ViewportSpec);
    m_AccumulationFBO.Create();
    m_FinalOutputFBO = Framebuffer(m_ViewportSpec);
    m_FinalOutputFBO.Create();

    m_BVHDebugShader    = std::make_unique<Shader>("shaders/debugVert.glsl", "shaders/debug.glsl");
    m_FinalOutputShader = std::make_unique<Shader>("shaders/vert.glsl", "shaders/post.glsl");
    m_AccumShader       = std::make_unique<Shader>("shaders/vert.glsl", "shaders/accumulation.glsl");
    m_PathTraceShader   = std::make_unique<Shader>("shaders/vert.glsl", "shaders/pt.glsl");

    m_Scene->SelectScene();
    m_BVH = std::make_unique<BVH>(m_Scene->primitives);

    int treeSize = m_BVH->CountNodes(m_BVH->bvh_root);

    // Setup BVH UBO
    glGenBuffers(1, &m_BVHBlockBuffer); 
    glBindBuffer(GL_UNIFORM_BUFFER, m_BVHBlockBuffer); 
    glBufferData(GL_UNIFORM_BUFFER, treeSize * sizeof(LinearBVH_Node), m_BVH->flat_root, GL_STATIC_DRAW); 
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_BVHBlockBuffer); 
    m_PathTraceShader->SetUBO("BVH", 0);

    // Setup PrimsBlock UBO
    glGenBuffers(1, &m_PrimsBlockBuffer); 
    glBindBuffer(GL_UNIFORM_BUFFER, m_PrimsBlockBuffer); 
    int mem = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(Light) + MAX_PRIMITIVES * sizeof(Primitive);
    glBufferData(GL_UNIFORM_BUFFER, mem, nullptr, GL_DYNAMIC_DRAW); 
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_PrimsBlockBuffer); 
    m_PathTraceShader->SetUBO("PrimsBlock", 1);

    // Setup SceneBlock UBO
    glGenBuffers(1, &m_SceneBlockBuffer); 
    glBindBuffer(GL_UNIFORM_BUFFER, m_SceneBlockBuffer); 
    glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneBlock), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_SceneBlockBuffer); 
    m_PathTraceShader->SetUBO("SceneBlock", 2);

    // Setup CameraBlock UBO
    glGenBuffers(1, &m_CameraBlockBuffer); 
    glBindBuffer(GL_UNIFORM_BUFFER, m_CameraBlockBuffer); 
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_CameraBlockBuffer); 
    m_PathTraceShader->SetUBO("CameraBlock", 3);
}

Renderer::~Renderer()
{
    m_PathTraceFBO.Destroy();
    m_AccumulationFBO.Destroy();
    m_FinalOutputFBO.Destroy();
}

void Renderer::UpdateBuffers()
{
    if (m_PathTraceShader->b_Reloaded)
    {
        std::cout << "Rebuilding BVH..." << std::endl;
        m_BVH->RebuildBVH(m_Scene->primitives);
        std::cout << "BVH Successfully Rebuilt" << std::endl;
        m_BVH->b_Rebuilt = true;
        
        // Reallocate memory for PrimsBlock
        int mem = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(Light) + MAX_PRIMITIVES * sizeof(Primitive);
        glBindBuffer(GL_UNIFORM_BUFFER, m_PrimsBlockBuffer);
        glBufferData(GL_UNIFORM_BUFFER, mem, nullptr, GL_DYNAMIC_DRAW); 
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_PrimsBlockBuffer); 
        m_PathTraceShader->SetUBO("PrimsBlock", 1);
        m_PathTraceShader->b_Reloaded = false;
    }

    // Update BVH Block only if rebuilt
    if (m_BVH->b_Rebuilt)
    {
        int treeSize = m_BVH->CountNodes(m_BVH->bvh_root);

        // Reallocate memory for BVH Block
        glBindBuffer(GL_UNIFORM_BUFFER, m_BVHBlockBuffer); 
        glBufferData(GL_UNIFORM_BUFFER, treeSize * sizeof(LinearBVH_Node), m_BVH->flat_root, GL_STATIC_DRAW); 
        glBindBuffer(GL_UNIFORM_BUFFER, 0); 

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_BVHBlockBuffer); 
        m_PathTraceShader->SetUBO("BVH", 0);
        m_BVH->b_Rebuilt = false;
    }

    // Update Scene Block
    glBindBuffer(GL_UNIFORM_BUFFER, m_SceneBlockBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SceneBlock), &m_Scene->Data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Update Camera Block
    glBindBuffer(GL_UNIFORM_BUFFER, m_CameraBlockBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraBlock), &m_Scene->Eye->params);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Update Prims Block
    glBindBuffer(GL_UNIFORM_BUFFER, m_PrimsBlockBuffer);
    int offset = 0;
    int n_Lights = (int) m_Scene->lights.size();
    int n_Primitives = (int) m_Scene->primitives.size();
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &n_Lights); 
    offset += sizeof(int);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(int), &n_Primitives); 
    offset += 3 * sizeof(int);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, n_Lights * sizeof(Light), m_Scene->lights.data()); 
    offset += (MAX_LIGHTS) * sizeof(Light);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, n_Primitives * sizeof(Primitive), m_Scene->primitives.data()); 
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::Render(uint32_t VAO, const ApplicationSettings& settings)
{
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f); 

    // First pass:
    // Render current frame to m_PathTraceFBO using m_AccumulationFBO's texture to continue accumulating samples
    // For first frame the texture will be empty and will not affect the output
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, m_AccumulationFBO.GetTextureID()); 

    m_PathTraceShader->Bind(); 
    m_PathTraceShader->SetUniformInt("u_AccumulationTexture", 0); 
    m_PathTraceShader->SetUniformInt("u_SampleIterations", m_SampleIterations); 
    m_PathTraceShader->SetUniformInt("u_SamplesPerPixel", m_Scene->samplesPerPixel); 
    m_PathTraceShader->SetUniformVec2("u_Resolution", float(m_ViewportWidth), float(m_ViewportHeight)); 
    m_PathTraceShader->SetUniformInt("u_BVHEnabled", int(settings.BVHEnabled));
    m_PathTraceShader->SetUniformInt("u_DebugBVHVisualisation", int(settings.debugBVHVisualisation));

    UpdateBuffers();

    m_PathTraceFBO.Bind(); 

    glClear(GL_COLOR_BUFFER_BIT); 
    glBindVertexArray(VAO); 
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 
    glBindVertexArray(0); 

    m_PathTraceFBO.Unbind(); 
    m_PathTraceShader->Unbind();

    // Second Pass:
    // This pass is used to copy the previous pass' output (m_PathTraceFBO) onto m_AccumulationFBO which will hold the data 
    // until used again for the first pass of the next frame
    m_AccumShader->Bind(); 
    m_AccumulationFBO.Bind(); 

    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, m_PathTraceFBO.GetTextureID()); 

    m_AccumShader->SetUniformInt("u_PathTraceTexture", 0); 
    m_AccumShader->SetUniformVec2("u_Resolution", float(m_ViewportWidth), float(m_ViewportHeight)); 

    glClear(GL_COLOR_BUFFER_BIT); 
    glBindVertexArray(VAO); 
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 
    glBindVertexArray(0); 

    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, m_AccumulationFBO.GetTextureID()); 

    m_AccumulationFBO.Unbind(); 
    m_AccumShader->Unbind();

    // Final pass:
    // Now use the texture from either of the previously used FBO and divide by the frame count
    m_FinalOutputShader->Bind(); 
    m_FinalOutputFBO.Bind(); 
    m_FinalOutputShader->SetUniformInt("u_PT_Texture", 0); 
    m_FinalOutputShader->SetUniformVec2("u_Resolution", float(m_ViewportWidth), float(m_ViewportHeight)); 
    m_FinalOutputShader->SetUniformInt("u_Tonemap", settings.tonemap);

    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(VAO); 
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 
    glBindVertexArray(0); 

    m_FinalOutputFBO.Unbind(); 
    m_FinalOutputShader->Unbind();
    
    if (b_DrawBVH)
    {
        m_FinalOutputFBO.Bind(); 
        glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
        m_BVHDebugShader->Bind();

        m_BVHDebugShader->SetUniformMat4("u_View", m_Scene->Eye->GetView());
        m_BVHDebugShader->SetUniformMat4("u_Projection", m_Scene->Eye->GetProjection());
        // m_BVHDebugShader->SetUniformVec2("u_Resolution", (float)ViewportWidth, (float)ViewportHeight);

        DrawTree(*m_BVHDebugShader, m_BVH->bvh_root, debugVAO, 0, BVHDepth);
        m_BVHDebugShader->Unbind();
        m_FinalOutputFBO.Unbind(); 
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

    shader.SetUniformMat4("u_Model", model);

    glBindVertexArray(vao); 
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0); 
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (GLvoid*)(4 * sizeof(uint32_t))); 
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, (GLvoid*)(8 * sizeof(uint32_t))); 
    glBindVertexArray(0); 

    // glBindVertexArray(vao);
    // glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    // glBindVertexArray(0); 
}

void DrawTree(Shader& shader, BVH_Node* node, uint32_t vao, int currentDepth, int terminationDepth)
{
    if (node == nullptr || currentDepth == terminationDepth + 1)
        return;
    
    currentDepth++;
    DrawBbox(shader, *node, vao);
    DrawTree(shader, node->left, vao, currentDepth, terminationDepth);
    DrawTree(shader, node->right, vao, currentDepth, terminationDepth);
}