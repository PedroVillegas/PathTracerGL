#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void OnResize();

    inline void SetClearColour(const glm::vec4& colour) { glClearColor(colour.r, colour.g, colour.b, colour.a); }
    inline void Clear() { glClear(GL_COLOR_BUFFER_BIT); }

    void OnUIPreRender();
    void OnUIRender();
    void OnPreRender();
    void OnRender();

private:
    unsigned int m_ViewportWidth = 0, m_ViewportHeight = 0;
};