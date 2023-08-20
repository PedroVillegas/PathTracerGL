#pragma once

struct FramebufferSpec
{
    uint32_t width, height;
};

class Framebuffer
{
private:
    uint32_t m_ID = 0;
    uint32_t m_TextureID = 0;
    FramebufferSpec m_Spec;

public:
    Framebuffer();
    Framebuffer(FramebufferSpec& FBspec);
    ~Framebuffer();
    
    void OnResize(uint32_t width, uint32_t height);

    void Bind() const;
    void Unbind() const;
    void Create();
    void Destroy();

    void SetWidth(uint32_t width);
    void SetHeight(uint32_t height);

    uint32_t GetWidth() { return m_Spec.width; }
    uint32_t GetHeight() { return m_Spec.height; }
    FramebufferSpec GetSpec() { return m_Spec; }
    uint32_t GetTextureID() { return m_TextureID; }
};
