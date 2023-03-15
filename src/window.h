#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "consoleLogger.h"

class Window
{
public:
    Window() {}
    Window(const char* title, uint width, uint height);
    ~Window();
    void ProcessInput(int& SelectedIdx, int ObjectCount);
    void Clear() const;
    void Update() const;
    bool Closed() const;

    GLFWwindow* GetWindow() const { return m_Window; }
    uint GetWidth() const { return m_Width; }
    void SetWidth(uint width);
    uint GetHeight() const { return m_Height; }
    void SetHeight(uint height);
private:
    bool Init();
private:
    const char* m_Title;
    uint m_Width, m_Height;
    GLFWwindow* m_Window;
};
