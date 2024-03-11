#pragma once
#include <glad/glad.h>
#include <iostream>
#include <signal.h>


class Framebuffer
{
public:
    Framebuffer() = default;
    Framebuffer(uint32_t width, uint32_t height)
        : m_ID(0)
        , m_TextureID(0)
        , m_Width(width)
        , m_Height(height)
    {};
    ~Framebuffer() {};
    
    void OnResize(uint32_t width, uint32_t height);

    void Bind() const;
    void Unbind() const;
    void Create();
    void Destroy();
    uint32_t GetTextureID() { return m_TextureID; }

private:
    uint32_t m_ID;
    uint32_t m_TextureID;
    uint32_t m_Width;
    uint32_t m_Height;
};
