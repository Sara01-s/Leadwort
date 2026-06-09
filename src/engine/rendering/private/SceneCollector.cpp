#include "engine/rendering/public/SceneCollector.h"

#include "engine/core/public/Scene.h"
#include "engine/core/public/Entity.h"
#include "engine/components/public/Camera.h"
#include "engine/components/public/Renderer.h"
#include <ranges>

namespace Engine::Rendering {

void SceneCollector::FindRenderersInScene(const Core::Scene* scene) {
	m_Renderers.clear();

	for (const auto& entity : scene->GetEntities() | std::views::values) {
		for (auto* component : entity->GetAllComponents()) {
			if (auto* renderer = dynamic_cast<Components::Renderer*>(component)) {
				CORE_LOG("Renderer en: ", renderer->entity->name);
				m_Renderers.push_back(renderer);
			}
		}
	}
}

[[nodiscard]] RenderQueues SceneCollector::BuildRenderQueues(const Components::Camera* camera) const {
	RenderQueues result;

	for (auto* renderer : m_Renderers) {
		if (renderer->isVisible && camera->ShouldRender(renderer->entity)) {
			const auto slot = static_cast<std::uint8_t>(renderer->renderQueue);
			result[slot].push_back(renderer);
		}
	}

	return result;
}

} // namespace Engine::Rendering