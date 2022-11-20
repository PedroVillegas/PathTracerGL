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

    inline uint GetWidth() { return m_Spec.width; }
    inline uint GetHeight() { return m_Spec.height; }
    inline FramebufferSpec GetSpec() { return m_Spec; }
    inline uint GetTextureID() { return m_TextureID; }
};
