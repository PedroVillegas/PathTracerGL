#include "renderer.h"
#include <stdio.h>

Renderer::Renderer(Shader& shader, uint ViewportWidth, uint ViewportHeight)
    :
    m_Shader(&shader),
    m_ViewportWidth(ViewportWidth),
    m_ViewportHeight(ViewportHeight)
{
    m_ViewportSpec.width = m_ViewportWidth;
    m_ViewportSpec.height = m_ViewportHeight;
    m_ViewportFramebuffer = Framebuffer(m_ViewportSpec);
    //m_ViewportFramebuffer.Create();
}

Renderer::~Renderer()
{
    m_ViewportFramebuffer.Destroy();
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

    m_ViewportFramebuffer.OnResize(m_ViewportWidth, m_ViewportHeight);
}

void Renderer::Render(const Scene& scene, const Camera& camera, uint VAO, int frameCounter)
{
    m_Scene = &scene;
    m_Camera = &camera;

    m_Shader->Bind();

    m_ViewportFramebuffer.Create();
    m_ViewportFramebuffer.Bind();
    SetClearColour(glm::vec4(1.0f));
    Clear();

    m_Shader->SetUniformInt("u_FrameCounter", frameCounter);
    m_Shader->SetUniformVec3("u_LightDirection", m_Scene->lightDirection.x, m_Scene->lightDirection.y, m_Scene->lightDirection.z);
    m_Shader->SetUniformVec2("u_Resolution", float(m_ViewportWidth), float(m_ViewportHeight));
    m_Shader->SetUniformVec3("u_RayOrigin", m_Camera->GetPosition().x, m_Camera->GetPosition().y, m_Camera->GetPosition().z);
    m_Shader->SetUniformMat4("u_InverseProjection", m_Camera->GetInverseProjection());
    m_Shader->SetUniformMat4("u_InverseView", m_Camera->GetInverseView());
    m_Shader->SetUniformInt("u_PreviousFrame", 0);

    glBindVertexArray(VAO); GLCall;
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GLCall;
    glBindVertexArray(0); GLCall;

    m_Shader->Unbind();
    m_ViewportFramebuffer.Unbind();
}