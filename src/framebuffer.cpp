#include "framebuffer.h"

Framebuffer::Framebuffer(FramebufferSpec& FBspec)
{
    m_Spec = FBspec;
    // Create();
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &m_ID);
}

void Framebuffer::OnResize(unsigned int width, unsigned int height)
{
    if (width == m_Spec.width && height == m_Spec.height)
        return;

    m_Spec.width = width;
    m_Spec.height = height;
}

void Framebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
}

void Framebuffer::Create()
{
    glGenFramebuffers(1, &m_ID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ID); // Select m_ID as the framebuffer to be rendered to

    // Create colour texture size: width, height
    glGenTextures(1, &m_TextureID); // Generate texture with ID: m_TextureID
    glBindTexture(GL_TEXTURE_2D, m_TextureID); // Select texture as current 2D Texture
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, 300, 300, 0, GL_RGB, 
        GL_UNSIGNED_BYTE, nullptr
        ); // Build texture with specified dimensions
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Attach texture to the framebuffer
    // std::cout << "Colour Buffer: " << m_TextureID << std::endl;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0);

    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer has failed to complete!" << fboStatus <<  std::endl;
    else if (fboStatus == GL_FRAMEBUFFER_COMPLETE) {}
        // std::cout << "Framebuffer successfully completed!" << std::endl;
}

void Framebuffer::Destroy()
{
    glDeleteFramebuffers(1, &m_ID);
    glDeleteTextures(1, &m_TextureID);
}

void Framebuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Framebuffer::SetWidth(unsigned int width)
{
    m_Spec.width = width;
}

void Framebuffer::SetHeight(unsigned int height)
{
    m_Spec.height = height;
}
