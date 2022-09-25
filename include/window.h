#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Window
{
private:
    const char* m_Title;
    unsigned int m_Width, m_Height;
    GLFWwindow* m_Window;

public:
    Window(const char* title, unsigned int width, unsigned int height);
    ~Window();
    void ProcessInput() const;
    void Clear() const;
    void Update() const;
    bool Closed() const;

    inline GLFWwindow* GetWindow() const { return m_Window; }
    inline unsigned int GetWidth() const { return m_Width; }
    void SetWidth(unsigned int width);
    inline unsigned int GetHeight() const { return m_Height; }
    void SetHeight(unsigned int height);

private:
    bool Init();
};
