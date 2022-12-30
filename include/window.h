#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "camera.h"
#include "consoleLogger.h"

class Window
{
private:
    const char* m_Title;
    uint m_Width, m_Height;
    GLFWwindow* m_Window;

public:
    Window(const char* title, uint width, uint height);
    ~Window();
    void ProcessInput();
    void Clear() const;
    void Update() const;
    bool Closed() const;

    inline GLFWwindow* GetWindow() const { return m_Window; }
    inline uint GetWidth() const { return m_Width; }
    void SetWidth(uint width);
    inline uint GetHeight() const { return m_Height; }
    void SetHeight(uint height);

private:
    bool Init();
};
