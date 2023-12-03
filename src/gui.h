#pragma once

class Gui
{
public:
    Gui() {}
    Gui(Window& window);
    ~Gui();
    void Render(Renderer& renderer, Scene& scene, bool& vsync);
    void NewFrame();
private:
    void Init();
    void Shutdown();
    void SetupStyle();
    void CreateCameraWindow(Renderer& renderer, Scene& scene);
    void CreateSceneWindow(Renderer& renderer, Scene& scene);
    template <typename T> void EditObjectProperties(T& obj, Renderer& renderer, Scene& scene);
private:
    Window m_Window;
};