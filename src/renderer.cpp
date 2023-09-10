#include "imgui/imgui.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stdio.h>

#include "consoleLogger.h"
#include "renderer.h"

Renderer::Renderer(Shader& PathTracerShader, Shader& AccumShader, Shader& FinalOutputShader, uint32_t ViewportWidth, uint32_t ViewportHeight)
    :
    m_PathTraceShader(&PathTracerShader),
    m_AccumShader(&AccumShader),
    m_FinalOutputShader(&FinalOutputShader),
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
}

Renderer::~Renderer()
{
    m_PathTraceFBO.Destroy();
    m_AccumulationFBO.Destroy();
    m_FinalOutputFBO.Destroy();
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

void Renderer::Render(const Scene& scene, const Camera& camera, uint32_t VAO)
{
    SetClearColour(1.0f, 0.0f, 1.0f, 1.0f); GLCall;
    m_Scene = &scene;
    m_Camera = &camera;

    // First pass:
    // Render current frame to m_PathTraceFBO using m_AccumulationFBO's texture to continue accumulating samples
    // For first frame the texture will be empty and will not affect the output
    glActiveTexture(GL_TEXTURE0); GLCall;
    glBindTexture(GL_TEXTURE_2D, m_AccumulationFBO.GetTextureID()); GLCall;

    m_PathTraceShader->Bind(); GLCall;
    m_PathTraceShader->SetUniformInt("u_AccumulationTexture", 0); GLCall;
    m_PathTraceShader->SetUniformInt("u_SampleIterations", m_SampleIterations); GLCall;
    m_PathTraceShader->SetUniformInt("u_SamplesPerPixel", m_Scene->samplesPerPixel); GLCall;
    m_PathTraceShader->SetUniformInt("u_Depth", m_Scene->maxRayDepth); GLCall;
    m_PathTraceShader->SetUniformInt("u_Day", m_Scene->day); GLCall;
    // m_PathTraceShader->SetUniformInt("u_SelectedObjIdx", m_Scene->SelectedIdx); GLCall;
    m_PathTraceShader->SetUniformVec3("u_SunDir", m_Scene->lightDirection.x, m_Scene->lightDirection.y, m_Scene->lightDirection.z); GLCall;
    m_PathTraceShader->SetUniformFloat("u_Aperture", m_Camera->aperture); GLCall;
    m_PathTraceShader->SetUniformFloat("u_FocalLength", m_Camera->focal_length); GLCall;
    m_PathTraceShader->SetUniformVec2("u_Resolution", float(m_ViewportWidth), float(m_ViewportHeight)); GLCall;
    m_PathTraceShader->SetUniformVec3("u_RayOrigin", m_Camera->GetPosition().x, m_Camera->GetPosition().y, m_Camera->GetPosition().z); GLCall;
    m_PathTraceShader->SetUniformMat4("u_InverseProjection", m_Camera->GetInverseProjection()); GLCall;
    m_PathTraceShader->SetUniformMat4("u_InverseView", m_Camera->GetInverseView()); GLCall;

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

    m_SampleIterations++;
}
