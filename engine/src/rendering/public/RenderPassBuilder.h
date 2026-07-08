#pragma once

#include "RenderTexture.h"
#include <vector>

namespace Engine::Rendering {

class RenderPassBuilder {
public:
	void Read(RenderTexture* renderTexture) noexcept {
		m_Inputs.push_back(renderTexture);
	}

	void Write(RenderTexture* renderTexture) noexcept {
		m_Outputs.push_back(renderTexture);
	}

	void WriteDepth(RenderTexture* depthTexture) noexcept {
		m_Depth = depthTexture;
	}

	[[nodiscard]] std::span<RenderTexture* const> GetInputs()  const noexcept { return m_Inputs; }
	[[nodiscard]] std::span<RenderTexture* const> GetOutputs() const noexcept { return m_Outputs; }
	[[nodiscard]] RenderTexture* GetDepth() const noexcept { return m_Depth; }

private:
	std::vector<RenderTexture*> m_Inputs{};
	std::vector<RenderTexture*> m_Outputs{};
	RenderTexture* m_Depth { nullptr };
};

} // namespace Engine::Rendering