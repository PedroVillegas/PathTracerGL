#pragma once

#include "imgui/imgui.h"
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
    Renderer(Shader& PathTracerShader, Shader& AccumShader, Shader& FinalOutputShader, uint ViewportWidth, uint ViewportHeight);
    ~Renderer();

    void OnResize(uint width, uint height);

    void SetViewportWidth(uint width);
    void SetViewportHeight(uint height);

    uint GetViewportWidth() const { return m_ViewportWidth; }
    uint GetViewportHeight() const { return m_ViewportHeight; }
    Shader* GetShader() const { return m_PathTraceShader; }
    Framebuffer GetViewportFramebuffer() const { return m_FinalOutputFBO; }
    uint GetIterations() const { return m_SampleIterations; }

    void SetClearColour(const glm::vec4& colour) { glClearColor(colour.r, colour.g, colour.b, colour.a); GLCall; }
    void Clear() { glClear(GL_COLOR_BUFFER_BIT); GLCall; }

    void Render(const Scene& scene, const Camera& camera, uint VAO);
    void ResetSamples() { m_SampleIterations = 0; }

private:
    uint m_SampleIterations = 0;
    const Camera* m_Camera = nullptr;
    const Scene* m_Scene = nullptr;
    Shader* m_PathTraceShader = nullptr;
    Shader* m_AccumShader = nullptr;
    Shader* m_FinalOutputShader = nullptr;
    uint m_ViewportWidth, m_ViewportHeight = 0;
    FramebufferSpec m_ViewportSpec;
    Framebuffer m_PathTraceFBO;
    Framebuffer m_AccumulationFBO;
    Framebuffer m_FinalOutputFBO;
};