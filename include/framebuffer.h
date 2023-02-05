#pragma once

#include <glad/glad.h>
#include <iostream>
#include "consoleLogger.h"

struct FramebufferSpec
{
    uint width, height;
};

class Framebuffer
{
private:
    uint m_ID = 0;
    uint m_TextureID = 0;
    FramebufferSpec m_Spec;

public:
    Framebuffer();
    Framebuffer(FramebufferSpec& FBspec);
    ~Framebuffer();
    
    void OnResize(uint width, uint height);

    void Bind() const;
    void Unbind() const;
    void Create();
    void Destroy();

    void SetWidth(uint width);
    void SetHeight(uint height);

    uint GetWidth() { return m_Spec.width; }
    uint GetHeight() { return m_Spec.height; }
    FramebufferSpec GetSpec() { return m_Spec; }
    uint GetTextureID() { return m_TextureID; }
};
