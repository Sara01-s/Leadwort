#pragma once

#include "engine/components/public/Camera.h"
#include "engine/components/public/Renderer.h"
#include "engine/core/public/Scene.h"

#include <ranges>

namespace Engine::Rendering {

class SceneCollector {
public:
	void FindRenderersInScene(const Core::Scene* scene) {
		for (auto& queue : m_Cached) {
			queue.clear();
		}

		for (const auto& entity : scene->GetEntities() | std::views::values) {
			for (auto* component : entity->GetAllComponents()) {
				if (auto* renderer = dynamic_cast<Components::Renderer*>(component)) {
					m_Cached[static_cast<std::uint8_t>(renderer->renderQueue)].push_back(renderer);
					break;
				}
			}
		}
	}

	[[nodiscard]] RenderQueues BuildRenderQueues(const Components::Camera* camera) const {
		RenderQueues result;

		for (std::size_t i = 0; i < m_Cached.size(); ++i) {
			for (auto* r : m_Cached[i]) {
				if (r->isVisible && camera->ShouldRender(r->entity)) {
					result[i].push_back(r);
				}
			}
		}

		return result;
	}

	void Clear() {
		for (auto& queue : m_Cached) {
			queue.clear();
		}
	}

private:
	RenderQueues m_Cached;
};

} // namespace Engine::Rendering