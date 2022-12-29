#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "framebuffer.h"
#include "shader.h"
#include "camera.h"
#include "scene.h"
#include "consoleLogger.h"

class Renderer
{
public:
    Renderer(Shader& shader, uint ViewportWidth, uint ViewportHeight);
    ~Renderer();

    void OnResize(uint width, uint height);

    void SetViewportWidth(uint width);
    void SetViewportHeight(uint height);

    inline uint GetViewportWidth() const { return m_ViewportWidth; }
    inline uint GetViewportHeight() const { return m_ViewportHeight; }
    inline Shader* GetShader() const { return m_Shader; }
    inline Framebuffer GetViewportFramebuffer() const { return m_ViewportFramebuffer; }

    inline void SetClearColour(const glm::vec4& colour) { glClearColor(colour.r, colour.g, colour.b, colour.a); GLCall; }
    inline void Clear() { glClear(GL_COLOR_BUFFER_BIT); GLCall; }

    void Render(const Scene& scene, const Camera& camera, uint VAO, int frameCounter);

private:
    const Camera* m_Camera = nullptr;
    const Scene* m_Scene = nullptr;
    Shader* m_Shader = nullptr;
    uint m_ViewportWidth, m_ViewportHeight = 0;
    FramebufferSpec m_ViewportSpec;
    Framebuffer m_ViewportFramebuffer;
};