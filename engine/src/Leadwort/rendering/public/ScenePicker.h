#pragma once

#include <Leadwort/components/public/IRenderer.h>
#include <Leadwort/core/math/public/Ray.h>
#include <Leadwort/core/public/Entity.h>
#include <Leadwort/core/public/Scene.h>
#include <Leadwort/rendering/public/CoordinateSystem.h>

#include <limits>
#include <ranges>

namespace Leadwort::Rendering {

class ScenePicker {
public:
	[[nodiscard]] static EntityID Pick(const Ray& ray, const Core::Scene& scene) {
		EntityID closestEntity { Core::Entity::ROOT_ENTITY_ID };
		float closestT { std::numeric_limits<float>::max() };

		for (const auto& entity : scene.GetEntityMap() | std::views::values) {
			for (auto* component : entity->GetAllComponents()) {
				const auto* renderer { dynamic_cast<Components::IRenderer*>(component) };
				if (!renderer || !renderer->isVisible) {
					continue;
				}

				const auto localAABB { renderer->GetAABB() };
				if (!localAABB) {
					continue;
				}

				const AABB worldAABB { localAABB->Transformed(
					CoordinateSystem::CalculateModelMatrix(entity->GetTransform())
				)};

				if (const auto hit { ray.IntersectsAABB(worldAABB) }) {
					if (*hit < closestT) {
						closestT = *hit;
						closestEntity = entity->GetID();
					}
				}
			}
		}

		return closestEntity;
	}
};

} // namespace Leadwort::Rendering