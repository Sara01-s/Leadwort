#include "engine/components/public/MeshRenderer.h"

#include "engine/components/behaviours/public/DirectionaLight.h"
#include "engine/components/public/Camera.h"
#include "engine/components/public/Transform.h"
#include "engine/core/public/Entity.h"
#include "engine/rendering/public/MatrixUtils.h"
#include "engine/systems/public/LightingSystem.h"
#include "engine/utils/public/Logger.h"

namespace Engine::Components {

void MeshRenderer::Render(const Camera* camera) {
	CORE_ASSERT(entity->transform, "MeshRenderer::Render: Entity transform is null.");

	if (!mesh || !mesh->GetMaterial()) {
		return;
	}

	auto* material = mesh->GetMaterial();

	const auto model = Rendering::MatrixUtils::CalculateModelMatrix(*entity->transform);
	material->SetMat4("_ModelMatrix", model);

	if (const auto* sun = Systems::LightingSystem::Get().GetDirectionalLight()) {
		CORE_ASSERT(sun->entity,            "MeshRenderer::Render: Sun entity is null.");
		CORE_ASSERT(sun->entity->transform, "MeshRenderer::Render: Sun transform is missing.");

		material->SetVec3 ("_LightDirection", sun->entity->transform->GetForward());
		material->SetFloat("_LightIntensity", sun->intensity);
		material->SetColor3("_LightColor", sun->color);
	}

	material->Bind();
	mesh->Draw();
	material->Unbind();
}

} // namespace Engine::Components