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
	CORE_ASSERT(&GetEntity().GetTransform(), "MeshRenderer::Render: Entity transform is null.");
	CORE_ASSERT(mesh, "MeshRenderer::Render: Mesh is null.");

	auto* activeMaterial = m_OverrideMaterial
		? m_OverrideMaterial.get()
		: mesh->GetMaterial();

	CORE_ASSERT(activeMaterial, "MeshRenderer::Render: Material is null.");

	const Mat4 modelMatrix = Rendering::MatrixUtils::CalculateModelMatrix(GetEntity().GetTransform());
	const Mat3 normalMatrix = Transpose(Inverse(modelMatrix.ToMat3()));

	activeMaterial->GetShader().Bind();
	activeMaterial->GetShader().SetUniform("_ModelMatrix", modelMatrix);
	activeMaterial->GetShader().SetUniform("_NormalMatrix", normalMatrix);

	if (const auto* sun = Systems::LightingSystem::Get().GetDirectionalLight()) {
		const Vec3 dir = sun->GetEntity().GetTransform().GetForward();

		activeMaterial->SetVec3("_LightDirection", dir);
		activeMaterial->SetFloat("_LightIntensity", sun->intensity);
		activeMaterial->SetColor3("_LightColor", sun->color);
	}
	else {
		CORE_WARN("No directional light found!");
	}

	activeMaterial->Bind();
	mesh->Render();
	activeMaterial->Unbind();
}

} // namespace Engine::Components