#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "framebuffer.h"
#include "shader.h"
#include "camera.h"
#include "scene.h"
class Renderer
{
public:
    Renderer(Shader shader, uint ViewportWidth, uint ViewportHeight);
    ~Renderer();

    void OnResize(uint width, uint height);

    void SetViewportWidth(uint width);
    void SetViewportHeight(uint height);

    inline uint GetViewportWidth() { return m_ViewportWidth; }
    inline uint GetViewportHeight() { return m_ViewportHeight; }
    inline Shader GetShader() { return m_Shader; }
    inline Framebuffer GetViewportFramebuffer() { return m_ViewportFramebuffer; }

    inline void SetClearColour(const glm::vec4& colour) { glClearColor(colour.r, colour.g, colour.b, colour.a); }
    inline void Clear() { glClear(GL_COLOR_BUFFER_BIT); }

    void Render(const Scene& scene, const Camera& camera, uint VAO);

private:
    Camera m_Camera;
    Scene m_Scene;
    Shader m_Shader;
    uint m_ViewportWidth, m_ViewportHeight = 0;
    FramebufferSpec m_ViewportSpec;
    Framebuffer m_ViewportFramebuffer;
};