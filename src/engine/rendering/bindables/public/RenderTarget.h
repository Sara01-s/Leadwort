#pragma once

#include "Bindable.h"
#include "engine/core/math/public/Vec2.h"

namespace Engine::Rendering::Bindables {

class RenderTarget : public Bindable {
public:
    RenderTarget(int width, int height, bool hdr = false);
    ~RenderTarget() override;

    RenderTarget(const RenderTarget&) = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;

    void Resize(int newWidth, int newHeight);

    void Bind() const noexcept override;
    void Unbind() const noexcept override;

	[[nodiscard]] constexpr Vec2 GetResolution() const noexcept { return Vec2(m_Width, m_Height); }
	[[nodiscard]] constexpr float GetAspectRatio() const noexcept { return static_cast<float>(m_Width) / static_cast<float>(m_Height); }
    [[nodiscard]] constexpr int GetWidth() const { return m_Width; }
    [[nodiscard]] constexpr int GetHeight() const { return m_Height; }
    [[nodiscard]] constexpr unsigned int GetTextureGpuID() const { return m_TextureGpuID; }

private:
    void Setup();
	void Clear() const;

private:
    int m_Width;
    int m_Height;
    bool m_Hdr;
    
    unsigned int m_FramebufferID = 0;
    unsigned int m_TextureGpuID = 0;
	unsigned int m_DepthRBO = 0;
};

} // namespace Engine::Rendering::Bindables