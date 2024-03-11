#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glm/gtc/type_ptr.hpp>

#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "scene.h"
#include "utils.h"


class Gui
{
public:
    Gui() {}
    Gui(Window& window);
    ~Gui();
    void Render(Renderer& renderer, Scene& scene, ApplicationSettings& settings, const std::vector<std::string>& envMaps);
    void NewFrame();

private:
    void Init();
    void Shutdown();
    void SetupStyle();
    void CreateCameraWindow(Renderer& renderer, Scene& scene);
    void CreateSceneWindow(Renderer& renderer, Scene& scene, const std::vector<std::string>& envMaps);

private:
    Window m_Window;
};