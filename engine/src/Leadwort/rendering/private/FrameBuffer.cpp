#include "../public/rendergraph/FrameBuffer.h"

#include <Leadwort/utils/public/Logger.h>

namespace Leadwort::Rendering::RG {

FrameBuffer::FrameBuffer(
	const std::span<RenderTexture* const> colorAttachments,
	const std::optional<RenderTexture*> depth
)
    : m_Depth(depth)
{
    LW_ASSERT(!colorAttachments.empty() || depth.has_value(),
        "Framebuffer: must have at least one attachment.");

    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    // Color attachments
    std::vector<GLenum> drawBuffers{};
    drawBuffers.reserve(colorAttachments.size());

    for (GLuint i = 0; i < static_cast<GpuID>(colorAttachments.size()); i++) {
        RenderTexture* renderTexture { colorAttachments[i] };
        LW_ASSERT(renderTexture && renderTexture->IsValid(), "Framebuffer: null or invalid RenderTexture.");
        LW_ASSERT(!renderTexture->IsDepth(), "Framebuffer: depth texture passed as color attachment.");

        glFramebufferTexture2D(GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D,
            renderTexture->GetGpuID(),
            0
        );

        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
        m_ColorAttachments.push_back(renderTexture);
    }

    if (!drawBuffers.empty()) {
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    }

    // Depth / stencil attachment
	if (const auto* depthPtr { depth.value_or(nullptr) }) {
		LW_ASSERT(depthPtr->IsDepth(), "Framebuffer: non-depth texture passed as depth attachment.");
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthPtr->GetGpuID(), 0);
	}

    LW_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer: incomplete framebuffer object.");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer() {
    if (m_FBO != 0) {
        glDeleteFramebuffers(1, &m_FBO);
        m_FBO = 0;
    }
}

void FrameBuffer::Bind() const noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glViewport(0, 0, this->GetWidth(), this->GetHeight());
}

void FrameBuffer::Unbind() noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int FrameBuffer::GetWidth() const noexcept {
    if (!m_ColorAttachments.empty()) {
	    return m_ColorAttachments[0]->GetWidth();
    }

    if (m_Depth.has_value()) {
	    return m_Depth.value()->GetWidth();
    }

    return 0;
}

int FrameBuffer::GetHeight() const noexcept {
    if (!m_ColorAttachments.empty()) {
	    return m_ColorAttachments[0]->GetHeight();
    }

    if (m_Depth.has_value()) {
	    return m_Depth.value()->GetHeight();
    }

    return 0;
}

Vec2 FrameBuffer::GetResolution() const noexcept {
    return Vec2(GetWidth(), GetHeight());
}

RenderTexture* FrameBuffer::GetColorAttachment(const std::size_t index) const noexcept {
    LW_ASSERT(index < m_ColorAttachments.size(), "Framebuffer: the attachment index is out of range.");
    return m_ColorAttachments[index];
}

} // namespace Engine::Rendering::Bindables