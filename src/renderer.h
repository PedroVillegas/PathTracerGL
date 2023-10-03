#pragma once

#include "shader.h"
#include "framebuffer.h"
#include "scene.h"
#include "camera.h"
#include "bvh.h"

class Renderer
{
public:
    Renderer() {}
    Renderer(
        uint32_t ViewportWidth,
        uint32_t ViewportHeight,
        Scene* scene,
        Camera* camera);
    ~Renderer();

    void OnResize(uint32_t width, uint32_t height);

    void SetViewportWidth(uint32_t width);
    void SetViewportHeight(uint32_t height);

    uint32_t GetViewportWidth() const { return m_ViewportWidth; }
    uint32_t GetViewportHeight() const { return m_ViewportHeight; }
    Shader* GetShader() const { return m_PathTraceShader; }
    Framebuffer GetViewportFramebuffer() const { return m_FinalOutputFBO; }
    uint32_t GetIterations() const { return m_SampleIterations; }

    void SetClearColour(float r, float g, float b, float a) { glClearColor(r, g, b, a); }
    void Clear() { glClear(GL_COLOR_BUFFER_BIT); }

    void UpdateBuffers();
    void Render(const Scene& scene, const Camera& camera, uint32_t VAO);
    void ResetSamples() { m_SampleIterations = 0; }
public:
    BVH* m_BVH = nullptr;
    int BVHDepth = 0;
    bool b_Pause = false;
    bool b_DrawBVH = false;
    uint32_t debugVAO = 0;
private:
    uint32_t m_SampleIterations = 0;
    Camera* m_Camera = nullptr;
    Scene* m_Scene = nullptr;
    uint32_t m_CameraBlockBuffer;
    uint32_t m_SceneBlockBuffer;
    uint32_t m_PrimsBlockBuffer;
    uint32_t m_BVHBlockBuffer;

    Shader* m_PathTraceShader = nullptr;
    Shader* m_AccumShader = nullptr;
    Shader* m_FinalOutputShader = nullptr;
    Shader* m_BVHDebugShader = nullptr;
    uint32_t m_ViewportWidth, m_ViewportHeight = 0;
    FramebufferSpec m_ViewportSpec;
    Framebuffer m_PathTraceFBO;
    Framebuffer m_AccumulationFBO;
    Framebuffer m_FinalOutputFBO;
};