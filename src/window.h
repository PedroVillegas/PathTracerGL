#pragma once

#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "consoleLogger.h"

class Window
{
public:
    Window() {}
    Window(const char* title, uint32_t width, uint32_t height);
    ~Window();
    void ProcessInput();
    void Clear() const;
    void Update() const;
    bool Closed() const;

    GLFWwindow* GetWindow() const { return m_Window; }
    uint32_t GetWidth() const { return m_Width; }
    void SetWidth(uint32_t width);
    uint32_t GetHeight() const { return m_Height; }
    void SetHeight(uint32_t height);
private:
    bool Init();
private:
    const char* m_Title;
    uint32_t m_Width, m_Height;
    GLFWwindow* m_Window;
};
