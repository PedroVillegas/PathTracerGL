#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "scene.h"

class Gui
{
public:
    Gui() {}
    Gui(Window& window);
    ~Gui();
    void Render(Renderer& renderer, Camera& camera, Scene& scene, bool& vsync);
    void NewFrame();
private:
    void Init();
    void Shutdown();
    void SetupStyle();
    void CreateCameraWindow(Renderer& renderer, Camera& camera);
    void CreateSceneWindow(Renderer& renderer, const Camera& camera, Scene& scene);
    template <typename T> void EditObjectProperties(T& obj, Renderer& renderer, const Camera& camera);
private:
    Window m_Window;
};