#pragma once

#include <Leadwort/core/math/public/Vec2.h>

#include <glad/glad.h>

namespace Leadwort::Rendering {

// Possesses one GPU texture (for color or depth).
// It does not possess an FBO; use Framebuffer to combine textures into a render target.
class RenderTexture {
public:
    enum class Format {
        RGB8,             // LDR color
        RGBA8,            // LDR color + alpha
        RGBA16F,          // HDR color
        Depth24Stencil8,  // Depth + stencil (not sampleable as color)
    };

public:
    RenderTexture(int width, int height, Format format);
    ~RenderTexture();

    RenderTexture(const RenderTexture&)            = delete;
    RenderTexture& operator=(const RenderTexture&) = delete;

    // Recreates the texture at the new size. Invalidates any Framebuffer
    // that references this texture, those must be rebuilt after resizing.
    void Resize(int newWidth, int newHeight);

    // Binds as a sampler at the given texture unit.
    void BindAsInput(GLint slot) const noexcept;

    [[nodiscard]] GLuint  GetGpuID()      const noexcept { return m_TextureGpuID; }
    [[nodiscard]] int     GetWidth()      const noexcept { return m_Width; }
    [[nodiscard]] int     GetHeight()     const noexcept { return m_Height; }
    [[nodiscard]] Format  GetFormat()     const noexcept { return m_Format; }
    [[nodiscard]] Vec2    GetResolution() const noexcept { return Vec2(m_Width, m_Height); }
    [[nodiscard]] float   GetAspectRatio() const noexcept {
        return static_cast<float>(m_Width) / static_cast<float>(m_Height);
    }
    [[nodiscard]] bool IsDepth() const noexcept {
        return m_Format == Format::Depth24Stencil8;
    }
    [[nodiscard]] bool IsValid() const noexcept { return m_TextureGpuID != 0; }

private:
    void Setup();
    void Clear() noexcept;

    static void GetGLFormats(Format format, GLint& outInternal, GLenum& outFormat, GLenum& outType) noexcept;

private:
    int    m_Width{};
    int    m_Height{};
    Format m_Format{};
    GLuint m_TextureGpuID{ 0 };
};

} // namespace Engine::Rendering::Bindables