#pragma once

#include "../../core/public/Core.h"
#include "FrameBuffer.h"
#include "RenderContext.h"
#include "RenderPass.h"
#include "RenderPassBuilder.h"

namespace Leadwort::Core { class Entity; }

namespace Leadwort::Rendering {

class RenderGraph final {
private:
	struct PassEntry {
		Unique<RenderPass> pass{};
		Unique<FrameBuffer> frameBuffer{};
		std::vector<RenderTexture*> inputs{};
	};

public:
	void AddPass(Unique<RenderPass> pass) noexcept {
		m_IsCompiled = false;
		m_Passes.push_back(PassEntry { std::move(pass), nullptr, {} });
	}

	void Compile() noexcept;
	void Execute(Components::Camera& camera, RenderQueues& queues,
				 const Core::Entity* highlightedEntity) const noexcept;

	void Clear() noexcept {
		m_IsCompiled = false;
		m_Passes.clear();
	}

	[[nodiscard]] std::size_t GetPassCount() const noexcept { return m_Passes.size(); }

private:
	bool m_IsCompiled { false };
	std::vector<PassEntry> m_Passes{};
};

} // namespace Engine::Rendering