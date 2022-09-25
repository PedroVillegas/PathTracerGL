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
    
    void OnResize(unsigned int width, unsigned int height);

    void Bind() const;
    void Unbind() const;
    void Create();
    void Destroy();

    void SetWidth(unsigned int width);
    void SetHeight(unsigned int height);

    inline unsigned int GetWidth() { return m_Spec.width; }
    inline unsigned int GetHeight() { return m_Spec.height; }

    inline unsigned int GetTextureID() { return m_TextureID; }
};
