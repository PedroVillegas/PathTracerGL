#pragma once

#include <memory>
#include "shader.h"
#include "framebuffer.h"
#include "scene.h"
#include "camera.h"
#include "bvh.h"
#include "utils.h"
#include "lodepng.h"
#include "texture.h"
#include "stb/stb_image.h"


class Renderer
{
public:
    Renderer(uint32_t ViewportWidth, uint32_t ViewportHeight, Scene* scene);
    ~Renderer();

    void OnResize(uint32_t width, uint32_t height);

    Framebuffer GetViewportFramebuffer() const { return m_FinalOutputFBO; }
    uint32_t GetIterations() const { return m_SampleIterations; }
    Shader& GetShader() const { return *m_PathTraceShader; }

    void UpdateBuffers();
    void Render(uint32_t VAO, const ApplicationSettings& settings);
    void ResetSamples() { m_SampleIterations = 0; }
public:
    std::unique_ptr<BVH> m_BVH;

    int BVHDepth;
    bool hasPaused;
    bool shouldDrawBVH;
    uint32_t debugVAO;

private:
    uint32_t m_ViewportWidth;
    uint32_t m_ViewportHeight;
    uint32_t m_SampleIterations;
    uint32_t m_CameraBlockBuffer;
    uint32_t m_SceneBlockBuffer;
    uint32_t m_PrimsBlockBuffer;
    uint32_t m_BVHBlockBuffer; 

    std::unique_ptr<Scene> m_Scene;
    std::unique_ptr<Shader> m_PathTraceShader;
    std::unique_ptr<Shader> m_AccumShader;
    std::unique_ptr<Shader> m_FinalOutputShader;
    std::unique_ptr<Shader> m_BVHDebugShader;

    Framebuffer m_PathTraceFBO;
    Framebuffer m_AccumulationFBO;
    Framebuffer m_FinalOutputFBO;

    uint32_t m_EnvMapTex;
};