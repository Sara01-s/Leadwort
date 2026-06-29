#include "engine/rendering/public/SceneCollector.h"

#include "engine/components/public/Camera.h"
#include "engine/components/public/Renderer.h"
#include "engine/core/math/public/Frustum.h"
#include "engine/core/public/Entity.h"
#include "engine/core/public/Scene.h"
#include "engine/rendering/public/MatrixUtils.h"

#include <ranges>

namespace Engine::Rendering {

void SceneCollector::FindRenderersInScene(const Core::Scene& scene) {
	m_Renderers.clear();

	for (const auto& entity : scene.GetEntities() | std::views::values) {
		for (auto* component : entity->GetAllComponents()) {
			if (auto* renderer = dynamic_cast<Components::Renderer*>(component)) {
				CORE_LOG("SceneCollector: Renderer found in the current scene: ", renderer->GetEntity().name);
				m_Renderers.push_back(renderer);
			}
		}
	}
}

[[nodiscard]]
RenderQueues SceneCollector::BuildRenderQueues(const Components::Camera& camera) const {
	RenderQueues result{};

	const Frustum frustum = Frustum::FromViewProjection(
		camera.GetProjectionMatrix() * camera.GetViewMatrix()
	);

	for (auto* renderer : m_Renderers) {
		if (!renderer->isVisible || !camera.ShouldRender(renderer->GetEntity())) {
			continue;
		}

		if (const auto aabb = renderer->GetAABB()) {
			const AABB worldAABB = aabb->Transformed(
				MatrixUtils::CalculateModelMatrix(renderer->GetEntity().GetTransform())
			);

			if (!frustum.Intersects(worldAABB)) {
				continue;
			}
		}

		result[static_cast<std::uint8_t>(renderer->renderQueue)].push_back(renderer);
	}

	return result;
}

} // namespace Engine::Rendering