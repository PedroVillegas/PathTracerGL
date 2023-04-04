#include "renderer.h"
#include <stdio.h>

Renderer::Renderer(Shader& PathTracerShader, Shader& AccumShader, Shader& FinalOutputShader, uint ViewportWidth, uint ViewportHeight)
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

void Renderer::SetViewportWidth(uint width)
{
    m_ViewportWidth = width;
}

void Renderer::SetViewportHeight(uint height)
{
    m_ViewportWidth = height;
}

void Renderer::OnResize(uint width, uint height)
{
    if (width == m_ViewportWidth && height == m_ViewportHeight)
        return;

    m_ViewportWidth = width;
    m_ViewportHeight = height;

    m_AccumulationFBO.OnResize(m_ViewportWidth, m_ViewportHeight);
    m_PathTraceFBO.OnResize(m_ViewportWidth, m_ViewportHeight);
    m_FinalOutputFBO.OnResize(m_ViewportWidth, m_ViewportHeight);
    ResetSamples();
}

void Renderer::Render(const Scene& scene, const Camera& camera, uint VAO)
{
    SetClearColour(glm::vec4(0.0f));
    m_Scene = &scene;
    m_Camera = &camera;

    // First pass:
    // Render current frame to m_PathTraceFBO using m_AccumulationFBO's texture to continue accumulating samples
    // For first frame the texture will be empty and will not affect the output
    glActiveTexture(GL_TEXTURE0); GLCall;
    glBindTexture(GL_TEXTURE_2D, m_AccumulationFBO.GetTextureID()); GLCall;

    m_PathTraceShader->Bind();
    m_PathTraceShader->SetUniformInt("u_AccumulationTexture", 0);
    m_PathTraceShader->SetUniformInt("u_SampleIterations", m_SampleIterations);
    m_PathTraceShader->SetUniformInt("u_SamplesPerPixel", m_Scene->samplesPerPixel);
    m_PathTraceShader->SetUniformInt("u_Depth", m_Scene->maxRayDepth);
    m_PathTraceShader->SetUniformInt("u_Day", m_Scene->day);
    // m_PathTraceShader->SetUniformInt("u_SelectedObjIdx", m_Scene->SelectedIdx);
    // m_PathTraceShader->SetUniformVec3("u_LightDir", m_Scene->lightDirection.x, m_Scene->lightDirection.y, m_Scene->lightDirection.z);
    m_PathTraceShader->SetUniformFloat("u_Aperture", m_Camera->aperture);
    m_PathTraceShader->SetUniformFloat("u_FocalLength", m_Camera->focal_length);
    m_PathTraceShader->SetUniformVec2("u_Resolution", float(m_ViewportWidth), float(m_ViewportHeight));
    m_PathTraceShader->SetUniformVec3("u_RayOrigin", m_Camera->GetPosition().x, m_Camera->GetPosition().y, m_Camera->GetPosition().z);
    // m_PathTraceShader->SetUniformVec3("u_ObjectCounts", m_Scene->spheres.size(), m_Scene->aabbs.size(), m_Scene->lights.size());
    m_PathTraceShader->SetUniformMat4("u_InverseProjection", m_Camera->GetInverseProjection());
    m_PathTraceShader->SetUniformMat4("u_InverseView", m_Camera->GetInverseView());
    // m_PathTraceShader->SetUniformMat4("u_ViewProjection", m_Camera->GetView() * m_Camera->GetProjection());

    m_PathTraceFBO.Bind();

    Clear();
    glBindVertexArray(VAO); GLCall;
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GLCall;
    glBindVertexArray(0); GLCall;

    m_PathTraceFBO.Unbind();
    m_PathTraceShader->Unbind();

    // Second Pass:
    // This pass is used to copy the previous pass' output (m_PathTraceFBO) onto m_AccumulationFBO which will hold the data 
    // until used again for the first pass of the next frame
    m_AccumShader->Bind();
    m_AccumulationFBO.Bind();

    glActiveTexture(GL_TEXTURE0); GLCall;
    glBindTexture(GL_TEXTURE_2D, m_PathTraceFBO.GetTextureID()); GLCall;

    m_AccumShader->SetUniformInt("u_PathTraceTexture", 0);
    m_AccumShader->SetUniformVec2("u_Resolution", float(m_ViewportWidth), float(m_ViewportHeight));

    Clear();
    glBindVertexArray(VAO); GLCall;
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GLCall;
    glBindVertexArray(0); GLCall;

    glActiveTexture(GL_TEXTURE0); GLCall;
    glBindTexture(GL_TEXTURE_2D, m_AccumulationFBO.GetTextureID()); GLCall;

    m_AccumShader->Unbind();
    m_AccumulationFBO.Unbind();

    // Final pass:
    // Now use the texture from either of the other FBO's and divide by the frame count
    m_FinalOutputShader->Bind();  
    m_FinalOutputFBO.Bind();
    m_FinalOutputShader->SetUniformInt("u_PT_Texture", 0);
    m_FinalOutputShader->SetUniformVec2("u_Resolution", float(m_ViewportWidth), float(m_ViewportHeight));

    Clear();
    glBindVertexArray(VAO); GLCall;
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GLCall;
    glBindVertexArray(0); GLCall;

    m_FinalOutputFBO.Unbind();
    m_FinalOutputShader->Unbind();  

    m_SampleIterations++;
}
