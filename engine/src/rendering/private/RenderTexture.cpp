#include "../public/RenderTexture.h"

namespace Engine::Rendering {

RenderTexture::RenderTexture(const int width, const int height, const Format format)
    : m_Width(width), m_Height(height), m_Format(format)
{
    Setup();
}

RenderTexture::~RenderTexture() {
    Clear();
}

void RenderTexture::Setup() {
    GLint  internal_format{};
    GLenum format{};
    GLenum type{};
    GetGLFormats(m_Format, internal_format, format, type);

    glGenTextures(1, &m_TextureGpuID);
    glBindTexture(GL_TEXTURE_2D, m_TextureGpuID);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_Width, m_Height, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderTexture::Clear() noexcept {
    if (m_TextureGpuID != 0) {
        glDeleteTextures(1, &m_TextureGpuID);
        m_TextureGpuID = 0;
    }
}

void RenderTexture::Resize(const int newWidth, const int newHeight) {
    Clear();
    m_Width  = newWidth;
    m_Height = newHeight;
    Setup();
}

void RenderTexture::BindAsInput(const GLint slot) const noexcept {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_TextureGpuID);
}

void RenderTexture::GetGLFormats(
    const Format format,
    GLint&  outInternal,
    GLenum& outFormat,
    GLenum& outType) noexcept
{
    switch (format) {
        case Format::RGB8:
            outInternal = GL_RGB8;
            outFormat   = GL_RGB;
            outType     = GL_UNSIGNED_BYTE;
            break;
        case Format::RGBA8:
            outInternal = GL_RGBA8;
            outFormat   = GL_RGBA;
            outType     = GL_UNSIGNED_BYTE;
            break;
        case Format::RGBA16F:
            outInternal = GL_RGBA16F;
            outFormat   = GL_RGBA;
            outType     = GL_FLOAT;
            break;
        case Format::Depth24Stencil8:
            outInternal = GL_DEPTH24_STENCIL8;
            outFormat   = GL_DEPTH_STENCIL;
            outType     = GL_UNSIGNED_INT_24_8;
            break;
    }
}

} // namespace Engine::Rendering::Bindables