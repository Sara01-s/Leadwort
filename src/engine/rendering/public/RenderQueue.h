// engine/rendering/public/RenderQueue.h
#pragma once

#include <cstdint>
#include <array>
#include <vector>

namespace Engine::Components {
	class Renderer;
}

namespace Engine::Rendering {

enum class RenderQueue : std::uint8_t {
	Background = 0,
	Opaque = 1,
	AlphaTest = 2, // pixel discards
	Transparent = 3, // blend
	Overlay = 4, // ui

	Count, // Ignore.
};

constexpr std::size_t RENDER_QUEUES_COUNT = static_cast<std::size_t>(RenderQueue::Count);

// Non-owning pointers, renderers are owned by their entities.
using RenderQueues = std::array<std::vector<Components::Renderer*>, RENDER_QUEUES_COUNT>;

}