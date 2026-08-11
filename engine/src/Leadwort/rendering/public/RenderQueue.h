// engine/rendering/public/RenderQueue.h
#pragma once

#include <cstdint>
#include <array>
#include <vector>

namespace Leadwort::Components {
	class IRenderer;
}

namespace Leadwort::Rendering {

	enum class RenderQueue : std::uint8_t {
		Background = 0, // Skybox, grid, etc...
		Opaque = 1, // Geometry
		AlphaTest = 2, // pixel discards
		Transparent = 3, // blend
		Overlay = 4, // ui

		Count, // Ignore.
	};

	constexpr std::size_t RENDER_QUEUES_COUNT = static_cast<std::size_t>(RenderQueue::Count);

	class RenderQueues {
	public:
		[[nodiscard]]
		auto& operator[](RenderQueue queue) noexcept {
			return m_Queues[static_cast<std::size_t>(queue)];
		}

		[[nodiscard]]
		const auto& operator[](RenderQueue queue) const noexcept {
			return m_Queues[static_cast<std::size_t>(queue)];
		}

	private:
		// Non-owning pointers, renderers are owned by their entities.
		std::array<std::vector<Components::IRenderer*>, RENDER_QUEUES_COUNT> m_Queues{};
	};

}