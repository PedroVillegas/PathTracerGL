#pragma once

#include <string>
#include "utils.h"
#include "imgui/imgui.h"
#include "window.h"
#include "renderer.h"
#include "gui.h"
#include "scene.h"
#include "camera.h"
#include "utils.h"


class Application
{
public:
    Application(std::string title, uint32_t width, uint32_t height);
    ~Application();
    void Run();

private:
    void Setup();
    void Resize();
    void Update();
    void Render();

    std::string m_Title;
    uint32_t m_ViewportWidth;
    uint32_t m_ViewportHeight;

    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Gui> m_Gui;
    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<Scene> m_Scene;

    ApplicationSettings m_Settings;

    float m_DeltaTime = 0.f;
    uint32_t m_QuadVAO;
    uint32_t m_QuadVBO;
    uint32_t m_QuadIBO;
    uint32_t m_BVHDebugVAO;
    uint32_t m_BVHDebugVBO;
    uint32_t m_BVHDebugIBO;
};
