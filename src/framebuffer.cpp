#include <glad/glad.h>
#include <iostream>
#include <signal.h>


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

void Framebuffer::OnResize(uint32_t width, uint32_t height)
{
    m_Spec.width = width;
    m_Spec.height = height;
    Destroy();
    Create();
}

void Framebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_ID); 
}

void Framebuffer::Create()
{
    glGenFramebuffers(1, &m_ID); 
    glBindFramebuffer(GL_FRAMEBUFFER, m_ID);  // Select m_ID as the framebuffer to be rendered to

    // Create colour texture size: width, height
    glGenTextures(1, &m_TextureID);  // Generate texture with ID: m_TextureID
    glBindTexture(GL_TEXTURE_2D, m_TextureID);  // Select texture as current 2D Texture
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA32F, m_Spec.width, m_Spec.height, 0, GL_RGBA, 
        GL_FLOAT, nullptr
        );  // Build texture with specified dimensions
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    // Attach texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0); 

    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER); 
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Failed to complete Framebuffer! FBO Status: " << fboStatus <<  std::endl;
    else if (fboStatus == GL_FRAMEBUFFER_COMPLETE) {}
        // std::cout << "Successfully completed Framebuffer! Colour buffer ID: " << m_TextureID << std::endl;
    
    glBindTexture(GL_TEXTURE_2D, 0); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void Framebuffer::Destroy()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    glDeleteFramebuffers(1, &m_ID); 
    m_ID = 0;
    glDeleteTextures(1, &m_TextureID); 
    m_TextureID = 0;
}

void Framebuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void Framebuffer::SetWidth(uint32_t width)
{
    m_Spec.width = width;
}

void Framebuffer::SetHeight(uint32_t height)
{
    m_Spec.height = height;
}
