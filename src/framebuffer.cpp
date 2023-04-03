#include "framebuffer.h"

Framebuffer::Framebuffer()
{
}

Framebuffer::Framebuffer(FramebufferSpec& FBspec)
    : m_Spec(FBspec)
{
}

Framebuffer::~Framebuffer()
{
}

void Framebuffer::OnResize(uint width, uint height)
{
    m_Spec.width = width;
    m_Spec.height = height;
    Destroy();
    Create();
}

void Framebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_ID); GLCall;
}

void Framebuffer::Create()
{
    glGenFramebuffers(1, &m_ID); GLCall;
    glBindFramebuffer(GL_FRAMEBUFFER, m_ID); GLCall; // Select m_ID as the framebuffer to be rendered to

    // Create colour texture size: width, height
    glGenTextures(1, &m_TextureID); GLCall; // Generate texture with ID: m_TextureID
    glBindTexture(GL_TEXTURE_2D, m_TextureID); GLCall; // Select texture as current 2D Texture
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA32F, m_Spec.width, m_Spec.height, 0, GL_RGBA, 
        GL_FLOAT, nullptr
        ); GLCall; // Build texture with specified dimensions
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); GLCall;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); GLCall;

    // Attach texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0); GLCall;

    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER); GLCall;
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Failed to complete Framebuffer! FBO Status: " << fboStatus <<  std::endl;
    else if (fboStatus == GL_FRAMEBUFFER_COMPLETE) {}
        // std::cout << "Successfully completed Framebuffer! Colour buffer ID: " << m_TextureID << std::endl;
    
    glBindTexture(GL_TEXTURE_2D, 0); GLCall;
    glBindFramebuffer(GL_FRAMEBUFFER, 0); GLCall;
}

void Framebuffer::Destroy()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0); GLCall;
    glDeleteFramebuffers(1, &m_ID); GLCall;
    m_ID = 0;
    glDeleteTextures(1, &m_TextureID); GLCall;
    m_TextureID = 0;
}

void Framebuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0); GLCall;
}

void Framebuffer::SetWidth(uint width)
{
    m_Spec.width = width;
}

void Framebuffer::SetHeight(uint height)
{
    m_Spec.height = height;
}
