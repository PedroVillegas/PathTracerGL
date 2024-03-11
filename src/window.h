#pragma once

#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <glfw/include/GLFW/glfw3.h>
#include <iostream>
#include <string>


class Window
{
public:
    Window() = default;
    Window(std::string title, uint32_t width, uint32_t height);
    ~Window() { glfwTerminate(); }
    void ProcessInput();
    void Clear() const;
    void Update() const;
    bool Closed() const;

    GLFWwindow* GetWindow() const { return m_Window; }

private:
    bool Init();

    std::string m_Title;
    uint32_t m_Width;
    uint32_t m_Height;
    GLFWwindow* m_Window;
};
