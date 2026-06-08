#include "../public/RenderTarget.h"

#include <stdexcept>

namespace Engine::Rendering::Bindables {

RenderTarget::RenderTarget(const int width, const int height, const bool hdr)
	: m_Width(width), m_Height(height), m_Hdr(hdr)
{
	Setup();
}

RenderTarget::~RenderTarget() {
	Clear();
}

void RenderTarget::Setup() {
	glGenFramebuffers(1, &m_FramebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

	glGenTextures(1, &m_TextureGpuID);
	glBindTexture(GL_TEXTURE_2D, m_TextureGpuID);

	const GLint internalFormat = m_Hdr ? GL_RGBA16F : GL_RGB8;
	const GLenum format = m_Hdr ? GL_RGBA : GL_RGB;
	const GLenum dataType = m_Hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, format, dataType, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureGpuID, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("Incomplete FrameBuffer!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::Clear() const {
	if (m_FramebufferID != 0) glDeleteFramebuffers(1, &m_FramebufferID);
	if (m_TextureGpuID != 0) glDeleteTextures(1, &m_TextureGpuID);
}

void RenderTarget::Resize(const int newWidth, const int newHeight) {
	Clear();
	m_Width = newWidth;
	m_Height = newHeight;
	Setup();
}

void RenderTarget::Bind() const noexcept { glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID); }
void RenderTarget::Unbind() const noexcept { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

} // namespace Engine::Rendering::Bindables