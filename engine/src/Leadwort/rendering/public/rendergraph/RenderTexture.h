#pragma once

#include <Leadwort/core/math/public/Vec2.h>

#include <glad/glad.h>
#include <string>
#include <string_view>

namespace Leadwort::Rendering::RG {

// Possesses one GPU texture (for color or depth).
// It does not possess an FBO; use Framebuffer to combine textures into a render target.
class RenderTexture {
public:
    enum class Format {
        RGB8,             // LDR color
        RGBA8,            // LDR color + alpha
        RGBA16F,          // HDR color
        Depth24Stencil8,  // Depth + stencil (not sampleable as color)
    	ShadowDepth32F,   // Depth-only, sampleable as sampler2DShadow (shadow maps)
    };

public:
    // debugName is purely informational: it labels the resource in editor tooling
    // (Render Graph window) and has no effect on the GPU resource.
    RenderTexture(int width, int height, Format format, std::string debugName = {});
    ~RenderTexture();

    RenderTexture(const RenderTexture&)            = delete;
    RenderTexture& operator=(const RenderTexture&) = delete;

    // Recreates the texture at the new size. Invalidates any Framebuffer
    // that references this texture, those must be rebuilt after resizing.
    void Resize(int newWidth, int newHeight);

    // Binds as a sampler at the given texture unit.
    void BindAsInput(GLint slot) const noexcept;

    [[nodiscard]] constexpr int GetWidth()  const noexcept { return m_Width; }
    [[nodiscard]] constexpr int GetHeight() const noexcept { return m_Height; }
    [[nodiscard]] GLuint GetGpuID()      const noexcept { return m_TextureGpuID; }
    [[nodiscard]] Format GetFormat()     const noexcept { return m_Format; }
    [[nodiscard]] Vec2   GetResolution() const noexcept { return Vec2(m_Width, m_Height); }
    [[nodiscard]] bool   IsDepth() const noexcept { return m_Format == Format::Depth24Stencil8 || m_Format == Format::ShadowDepth32F; }
	[[nodiscard]] bool   IsShadowSampleable() const noexcept { return m_Format == Format::ShadowDepth32F; }
    [[nodiscard]] bool   IsValid() const noexcept { return m_TextureGpuID != 0; }
	[[nodiscard]] float  GetAspectRatio() const noexcept {
    	return static_cast<float>(m_Width) / static_cast<float>(m_Height);
    }
	[[nodiscard]] bool HasStencil() const noexcept {
    	return m_Format == Format::Depth24Stencil8;
    }

	[[nodiscard]] const std::string& GetDebugName() const noexcept { return m_DebugName; }
	void SetDebugName(std::string name) noexcept { m_DebugName = std::move(name); }

	[[nodiscard]] static constexpr std::string_view GetFormatName(const Format format) noexcept {
    	switch (format) {
    		case Format::RGB8:            return "RGB8";
    		case Format::RGBA8:           return "RGBA8";
    		case Format::RGBA16F:         return "RGBA16F";
    		case Format::Depth24Stencil8: return "Depth24Stencil8";
    		case Format::ShadowDepth32F:  return "ShadowDepth32F";
    	}

    	return "Unknown";
    }

private:
    void Setup();
    void Clear() noexcept;

    static void GetGLFormats(Format format, GLint& outInternal, GLenum& outFormat, GLenum& outType) noexcept;

private:
    int    m_Width{};
    int    m_Height{};
    Format m_Format{};
    GLuint m_TextureGpuID{ 0 };
    std::string m_DebugName{};
};

} // namespace Engine::Rendering::Bindables