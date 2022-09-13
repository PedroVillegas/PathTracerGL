#pragma once

#include <glad/glad.h>
#include <iostream>

struct FramebufferSpec
{
    unsigned int width, height;
};

class Framebuffer
{
private:
    unsigned int m_ID;
    unsigned int m_TextureID;
    FramebufferSpec m_Spec;

public:
    Framebuffer(FramebufferSpec& FBspec);
    ~Framebuffer();
    
    void Bind() const;
    void Unbind() const;
    void Create();

    inline unsigned int GetTextureID() { return m_TextureID; }
};