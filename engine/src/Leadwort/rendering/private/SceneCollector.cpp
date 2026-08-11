#include <Leadwort/components/public/Camera.h>
#include <Leadwort/components/public/IRenderer.h>
#include <Leadwort/core/math/public/Frustum.h>
#include <Leadwort/core/public/Entity.h>
#include <Leadwort/core/public/Scene.h>
#include <Leadwort/rendering/public/CoordinateSystem.h>
#include <Leadwort/rendering/public/SceneCollector.h>

#include <ranges>

namespace Leadwort::Rendering {

void SceneCollector::FindRenderersInScene(const Core::Scene& scene) {
	m_Renderers.clear();

	for (const auto& entity : scene.GetEntityMap() | std::views::values) {
		for (auto* component : entity->GetAllComponents()) {
			if (auto* renderer = dynamic_cast<Components::IRenderer*>(component)) {
				LW_LOG("SceneCollector: Renderer found in the current scene: ", renderer->GetEntity().name);
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
				CoordinateSystem::CalculateModelMatrix(renderer->GetEntity().GetTransform())
			);

			if (!frustum.Intersects(worldAABB)) {
				continue;
			}
		}

		result[renderer->renderQueue].push_back(renderer);
	}

	return result;
}

} // namespace Engine::Rendering