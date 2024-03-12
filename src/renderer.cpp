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
    : m_BVH(nullptr)
    , BVHDepth(0)
    , hasPaused(false)
    , shouldDrawBVH(false)
    , debugVAO(0)
    , m_ViewportWidth(ViewportWidth)
    , m_ViewportHeight(ViewportHeight)
    , m_SampleIterations(0)
    , m_CameraBlockBuffer(0)
    , m_SceneBlockBuffer(0)
    , m_PrimsBlockBuffer(0)
    , m_BVHBlockBuffer(0)
    , m_Scene(scene)
    , m_PathTraceShader(nullptr)
    , m_AccumShader(nullptr)
    , m_FinalOutputShader(nullptr)
    , m_BVHDebugShader(nullptr)
    , m_EnvMapTex(0)
{
    m_PathTraceFBO = Framebuffer(m_ViewportWidth, m_ViewportHeight);
    m_PathTraceFBO.Create();
    m_AccumulationFBO = Framebuffer(m_ViewportWidth, m_ViewportHeight);
    m_AccumulationFBO.Create();
    m_FinalOutputFBO = Framebuffer(m_ViewportWidth, m_ViewportHeight);
    m_FinalOutputFBO.Create();

    m_BVHDebugShader    = std::make_unique<Shader>(PATH_TO_SHADERS + "debugVert.glsl", PATH_TO_SHADERS + "debug.glsl");
    m_FinalOutputShader = std::make_unique<Shader>(PATH_TO_SHADERS + "vert.glsl", PATH_TO_SHADERS + "post.glsl");
    m_AccumShader       = std::make_unique<Shader>(PATH_TO_SHADERS + "vert.glsl", PATH_TO_SHADERS + "accumulation.glsl");
    m_PathTraceShader   = std::make_unique<Shader>(PATH_TO_SHADERS + "vert.glsl", PATH_TO_SHADERS + "pt.glsl");

    m_Scene->SelectScene();
    m_BVH = std::make_unique<BVH>(m_Scene->primitives);

    // Setup BVH UBO
    int bvhBlockOffset = 1000 * sizeof(LinearBVH_Node);
    int bvhBlockMem = 1000 * sizeof(LinearBVH_Node) + MAX_PRIMITIVES * sizeof(int);
    glGenBuffers(1, &m_BVHBlockBuffer); 
    glBindBuffer(GL_UNIFORM_BUFFER, m_BVHBlockBuffer);
    glBufferData(GL_UNIFORM_BUFFER, bvhBlockMem, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, m_BVH->totalNodes * sizeof(LinearBVH_Node), m_BVH->flat_root);
    glBufferSubData(GL_UNIFORM_BUFFER, bvhBlockOffset, m_BVH->primitivesIndexBuffer.size() * sizeof(int), m_BVH->primitivesIndexBuffer.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_BVHBlockBuffer);

    // Setup PrimsBlock UBO
    int primsBlockMem = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(Light) + MAX_PRIMITIVES * sizeof(Primitive);
    glGenBuffers(1, &m_PrimsBlockBuffer); 
    glBindBuffer(GL_UNIFORM_BUFFER, m_PrimsBlockBuffer); 
    glBufferData(GL_UNIFORM_BUFFER, primsBlockMem, nullptr, GL_DYNAMIC_DRAW); 
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_PrimsBlockBuffer); 

    // Setup SceneBlock UBO
    glGenBuffers(1, &m_SceneBlockBuffer); 
    glBindBuffer(GL_UNIFORM_BUFFER, m_SceneBlockBuffer); 
    glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneBlock), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_SceneBlockBuffer); 

    // Setup CameraBlock UBO
    glGenBuffers(1, &m_CameraBlockBuffer); 
    glBindBuffer(GL_UNIFORM_BUFFER, m_CameraBlockBuffer); 
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_CameraBlockBuffer); 

    // Send Blue Noise 2d texture to PathTraceShader
    uint32_t BlueNoise;
    Texture* blueNoiseTex = new Texture;
    blueNoiseTex->LoadTexture(PROJECT_PATH + "assets/blueNoise.png");
    glGenTextures(1, &BlueNoise);
    glBindTexture(GL_TEXTURE_2D, BlueNoise);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, blueNoiseTex->width, blueNoiseTex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (*blueNoiseTex).data.data());
    delete blueNoiseTex;

    // Send Tony McMapface 3d texture to shader (post.glsl)
    uint32_t TonyMcMapfaceLUT;
    Texture* TonyMcMapfaceTex = new Texture;
    TonyMcMapfaceTex->LoadTexture(PROJECT_PATH + "assets/TonyMcMapfaceLUT.png");
    glGenTextures(1, &TonyMcMapfaceLUT);
    glBindTexture(GL_TEXTURE_3D, TonyMcMapfaceLUT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, 48, 48, 48, 0, GL_RGBA, GL_UNSIGNED_BYTE, (*TonyMcMapfaceTex).data.data());
    // The png LUT is 2304x48 with 48 layers --> 48x48x48
    delete TonyMcMapfaceTex;

    m_PathTraceShader->Bind();
    m_PathTraceShader->SetUBO("BVH", 0);
    m_PathTraceShader->SetUBO("PrimsBlock", 1);
    m_PathTraceShader->SetUBO("SceneBlock", 2);
    m_PathTraceShader->SetUBO("CameraBlock", 3);
    m_PathTraceShader->SetUniformInt("u_BlueNoise", 1);
    m_PathTraceShader->Unbind();

    m_FinalOutputShader->Bind();
    m_FinalOutputShader->SetUniformInt("u_TonyMcMapfaceLUT", 1);
    m_FinalOutputShader->Unbind();

    // Create texture for environment map
    if (m_Scene->envMap != nullptr)
    {
        glGenTextures(1, &m_EnvMapTex);
        glBindTexture(GL_TEXTURE_2D, m_EnvMapTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_Scene->envMap->width, m_Scene->envMap->height, 0, GL_RGBA, GL_FLOAT, m_Scene->envMap->data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Textures for post.glsl
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, TonyMcMapfaceLUT);

    // Textures for pt.glsl
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, BlueNoise);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_EnvMapTex);
}

Renderer::~Renderer()
{
    m_PathTraceFBO.Destroy();
    m_AccumulationFBO.Destroy();
    m_FinalOutputFBO.Destroy();
}

void Renderer::UpdateBuffers()
{
    if (m_PathTraceShader->hasReloaded)
    {
        m_BVH->RebuildBVH(m_Scene->primitives);
        
        // Reallocate memory for PrimsBlock
        int mem = sizeof(glm::vec4) + MAX_LIGHTS * sizeof(Light) + MAX_PRIMITIVES * sizeof(Primitive);
        glBindBuffer(GL_UNIFORM_BUFFER, m_PrimsBlockBuffer);
        glBufferData(GL_UNIFORM_BUFFER, mem, nullptr, GL_DYNAMIC_DRAW); 
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_PrimsBlockBuffer); 

        // Setup SceneBlock UBO
        glBindBuffer(GL_UNIFORM_BUFFER, m_SceneBlockBuffer); 
        glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneBlock), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_SceneBlockBuffer); 

        // Setup CameraBlock UBO
        glBindBuffer(GL_UNIFORM_BUFFER, m_CameraBlockBuffer); 
        glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_CameraBlockBuffer); 

        m_PathTraceShader->SetUBO("PrimsBlock", 1);
        m_PathTraceShader->SetUBO("SceneBlock", 2);
        m_PathTraceShader->SetUBO("CameraBlock", 3);
        m_PathTraceShader->SetUniformInt("u_BlueNoise", 1);
        m_PathTraceShader->hasReloaded = false;
    }

    // Update BVH Block only if rebuilt
    if (m_BVH->b_Rebuilt)
    {
        // Reallocate memory for BVH Block
        int bvhBlockOffset = 1000 * sizeof(LinearBVH_Node);
        int bvhBlockMem = 1000 * sizeof(LinearBVH_Node) + MAX_PRIMITIVES * sizeof(int);
        glBindBuffer(GL_UNIFORM_BUFFER, m_BVHBlockBuffer);
        glBufferData(GL_UNIFORM_BUFFER, bvhBlockMem, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, m_BVH->totalNodes * sizeof(LinearBVH_Node), m_BVH->flat_root);
        glBufferSubData(GL_UNIFORM_BUFFER, bvhBlockOffset, m_BVH->primitivesIndexBuffer.size() * sizeof(int), m_BVH->primitivesIndexBuffer.data());
        glBindBuffer(GL_UNIFORM_BUFFER, 0); 
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_BVHBlockBuffer); 

        m_PathTraceShader->SetUBO("BVH", 0);
        m_BVH->b_Rebuilt = false;
    }

    // Update Env Map Texture
    if (m_Scene->envMapHasChanged)
    {
        if (m_Scene->envMap != nullptr)
        {
            m_Scene->envMapHasChanged = false;
            if (m_EnvMapTex) glDeleteTextures(1, &m_EnvMapTex);

            glGenTextures(1, &m_EnvMapTex);
            glBindTexture(GL_TEXTURE_2D, m_EnvMapTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_Scene->envMap->width, m_Scene->envMap->height, 0, GL_RGBA, GL_FLOAT, m_Scene->envMap->data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);

            m_PathTraceShader->Bind();
            m_PathTraceShader->SetUniformInt("u_EnvMapTex", 2);
            m_PathTraceShader->Unbind();
        }
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
    int offset = 0;
    int n_Lights = (int) m_Scene->lights.size();
    int n_Primitives = (int) m_Scene->primitives.size();
    glBindBuffer(GL_UNIFORM_BUFFER, m_PrimsBlockBuffer);
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
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_EnvMapTex);

    m_PathTraceShader->Bind(); 
    m_PathTraceShader->SetUniformInt("u_EnvMapTex", 2);
    m_PathTraceShader->SetUniformInt("u_AccumulationTexture", 0); 
    m_PathTraceShader->SetUniformInt("u_SampleIterations", m_SampleIterations); 
    m_PathTraceShader->SetUniformInt("u_SamplesPerPixel", m_Scene->samplesPerPixel); 
    m_PathTraceShader->SetUniformVec2("u_Resolution", float(m_ViewportWidth), float(m_ViewportHeight)); 
    m_PathTraceShader->SetUniformInt("u_BVHEnabled", int(settings.enableBVH));
    m_PathTraceShader->SetUniformInt("u_DebugBVHVisualisation", int(settings.enableDebugBVHVisualisation));
    m_PathTraceShader->SetUniformInt("u_TotalNodes", m_BVH->totalNodes);
    m_PathTraceShader->SetUniformInt("u_UseBlueNoise", int(settings.enableBlueNoise));
    m_PathTraceShader->SetUniformFloat("u_EnvMapRotation", m_Scene->envMapRotation);

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
    m_FinalOutputShader->SetUniformInt("u_EnableCrosshair", int(settings.enableCrosshair));

    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(VAO); 
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 
    glBindVertexArray(0); 

    m_FinalOutputFBO.Unbind(); 
    m_FinalOutputShader->Unbind();
    
    if (shouldDrawBVH)
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

void Renderer::OnResize(uint32_t width, uint32_t height)
{
    if (width == m_ViewportWidth && height == m_ViewportHeight)
        return;

    m_ViewportWidth = width;
    m_ViewportHeight = height;

    m_AccumulationFBO.OnResize(m_ViewportWidth, m_ViewportHeight);
    m_PathTraceFBO.OnResize(m_ViewportWidth, m_ViewportHeight);
    m_FinalOutputFBO.OnResize(m_ViewportWidth, m_ViewportHeight);
    hasPaused = false;
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
}

void DrawTree(Shader& shader, BVH_Node* node, uint32_t vao, int currentDepth, int terminationDepth)
{
    if (node == nullptr || currentDepth == terminationDepth + 1)
        return;
    
    currentDepth++;
    //if (currentDepth == terminationDepth)
    DrawBbox(shader, *node, vao);
    DrawTree(shader, node->left, vao, currentDepth, terminationDepth);
    DrawTree(shader, node->right, vao, currentDepth, terminationDepth);
}