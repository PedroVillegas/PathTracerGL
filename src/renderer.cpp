#include "renderer.h"
#include <stdio.h>

Renderer::Renderer(Shader shader, uint ViewportWidth, uint ViewportHeight)
    :
    m_Camera(45.0f, 0.1f, 100.0f),
    m_Shader(shader),
    m_ViewportWidth(ViewportWidth),
    m_ViewportHeight(ViewportHeight)
{
    m_ViewportSpec.width = m_ViewportWidth;
    m_ViewportSpec.height = m_ViewportHeight;
    m_ViewportFramebuffer = Framebuffer(m_ViewportSpec);
    // std::cout << m_ViewportFramebuffer.GetWidth() << ", " << m_ViewportFramebuffer.GetHeight() << std::endl;
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

void Renderer::Render(const Scene& scene, const Camera& camera, uint VAO)
{
    m_Scene = scene;
    m_Camera = camera;

    // Bind custom framebuffer so frame can be rendered onto texture for ImGui::Image to display onto panel
    m_ViewportFramebuffer.Create();
    m_ViewportFramebuffer.Bind();
    Clear();

    m_Shader.Bind();
    /*
    for (uint i = 0; i < scene.Spheres.size(); i++)
    {
        float x = m_Scene.Spheres[i].Position.x;
        float y = m_Scene.Spheres[i].Position.y;
        float z = m_Scene.Spheres[i].Position.z;
        float r = m_Scene.Spheres[i].Albedo.x;
        float g = m_Scene.Spheres[i].Albedo.y;
        float b = m_Scene.Spheres[i].Albedo.z;
        m_Shader.SetUniformVec3("u_Spheres[" + std::to_string(i) + "]", x, y, z);
        m_Shader.SetUniformFloat("u_Spheres[" + std::to_string(i) + "]", m_Scene.Spheres[i].Radius);
        m_Shader.SetUniformVec3("u_Spheres[" + std::to_string(i) + "]", r, g, b);
    }*/

    m_Shader.SetUniformVec2("u_Resolution", m_ViewportWidth, m_ViewportHeight);
    m_Shader.SetUniformVec3("u_RayOrigin", m_Camera.GetPosition().x, m_Camera.GetPosition().y, m_Camera.GetPosition().z);
    m_Shader.SetUniformMat4("u_InverseProjection", m_Camera.GetInverseProjection());
    m_Shader.SetUniformMat4("u_InverseView", m_Camera.GetInverseView());

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    m_Shader.Unbind();    
    m_ViewportFramebuffer.Unbind();
}
