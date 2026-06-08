#pragma once

#include "engine/components/public/Renderer.h"
#include "engine/rendering/bindables/public/Material.h"
#include "engine/rendering/bindables/public/Mesh.h"
#include "engine/systems/public/LightingSystem.h"
#include "engine/systems/public/RenderSystem.h"
#include "engine/utils/public/PrimitiveMeshes.h"

#include <glm/glm.hpp>
#include <memory>

namespace Engine::Components {

class MeshRenderer final : public Renderer {
public:
	Rendering::Bindables::Mesh* mesh = Utils::PrimitiveMeshes::Get().Quad();

	float metallicIntensity  = 0.0f;
	float roughnessIntensity = 0.7f;

	Rendering::Bindables::Material* GetMaterial() const {
		return mesh->GetMaterial();
	}

	void SetMaterial(const std::shared_ptr<Rendering::Bindables::Material>& material) const {
		mesh->SetMaterial(material);
	}

	void Render(const Camera* camera) override {
		CORE_ASSERT(entity, "MeshRenderer::Render: Entity is null.");
		CORE_ASSERT(entity->transform, "MeshRenderer::Render: Entity transform is null.");
		CORE_ASSERT(mesh, "MeshRenderer::Render: Mesh data is null.");

		const auto model = Rendering::MatrixUtils::CalculateModelMatrix(*entity->transform);
		const auto normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

		auto* material = GetMaterial();
		CORE_ASSERT(material, "MeshRenderer::Render: Material is missing.");

		material->Bind();
		material->SetMat4("_ModelMatrix",  model);
		material->SetMat3("_NormalMatrix", normalMatrix);

		if (Systems::LightingSystem::Get().IsEnabled()) {
			const auto* sun = Systems::LightingSystem::Get().GetDirectionalLight();

			CORE_ASSERT(sun, "MeshRenderer::Render: Lighting enabled but no sun found.");
			CORE_ASSERT(sun->entity, "MeshRenderer::Render: Sun entity is null.");
			CORE_ASSERT(sun->entity->transform, "MeshRenderer::Render: Sun transform is missing.");

			material->SetVec3 ("_LightDirection", sun->entity->transform->GetForward());
			material->SetFloat("_LightIntensity", sun->intensity);
			material->SetColor3("_LightColor",    sun->color);
		}

		material->SetFloat("_MetallicIntensity",  metallicIntensity);
		material->SetFloat("_RoughnessIntensity", roughnessIntensity);

		mesh->Draw();
	}
};

} // namespace Engine::Components