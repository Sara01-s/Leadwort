#pragma once

#include "RenderTexture.h"

#include <glad/glad.h>
#include <optional>
#include <span>
#include <vector>

namespace Leadwort::Rendering {

// Owns a FBO that assembles N RenderTextures as color attachments
// and an optional depth/stencil attachment.
//
// Lifetime rule: all RenderTextures passed in must outlive this Framebuffer.
// After calling RenderTexture::Resize(), rebuild any Framebuffer that references it.
class FrameBuffer {
public:
    // color_attachments → GL_COLOR_ATTACHMENT0..N
    // depth             → GL_DEPTH_STENCIL_ATTACHMENT  (optional, pass std::nullopt to skip)
    //
    // All color attachments must have the same dimensions.
    explicit FrameBuffer(
        std::span<RenderTexture* const> colorAttachments,
        std::optional<RenderTexture*> depth = std::nullopt
    );

    ~FrameBuffer();

    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    void Bind() const noexcept;
	static void Unbind() noexcept;

    [[nodiscard]] int    GetWidth()      const noexcept;
    [[nodiscard]] int    GetHeight()     const noexcept;
    [[nodiscard]] Vec2   GetResolution() const noexcept;
    [[nodiscard]] GLuint GetFboID()      const noexcept { return m_FBO; }

    // Returns the color texture at the given attachment index.
    [[nodiscard]] RenderTexture* GetColorAttachment(std::size_t index) const noexcept;

    // Returns the depth texture, or nullptr if none was provided.
    [[nodiscard]] RenderTexture* GetDepthAttachment() const noexcept { return m_Depth.has_value() ? *m_Depth : nullptr; }

    [[nodiscard]] bool IsValid() const noexcept { return m_FBO != 0; }

private:
    GLuint m_FBO { 0 };
    std::vector<RenderTexture*> m_ColorAttachments{};
    std::optional<RenderTexture*> m_Depth { nullptr };
};

} // namespace Engine::Rendering::Bindables