#include <Leadwort/components/public/MeshRenderer.h>
#include <Leadwort/components/public/Transform.h>
#include <Leadwort/core/public/Entity.h>
#include <Leadwort/rendering/public/CoordinateSystem.h>
#include <Leadwort/rendering/public/DrawCommands.h>
#include <Leadwort/utils/public/Logger.h>

namespace Leadwort::Components {

	using namespace Rendering;

	void MeshRenderer::EmitDrawCommand(DrawCommandBuffer& drawCmdBuffer, const Camera& camera) const {
		LW_ASSERT(&GetEntity().GetTransform(), "MeshRenderer::Render: Entity transform is null.");
		LW_ASSERT(mesh, "MeshRenderer::Render: Mesh is null.");

		auto const& activeMaterial = mesh->GetMaterial();
		LW_ASSERT(activeMaterial, "MeshRenderer::Render: Material is null.");

		const Mat4  modelMatrix = CoordinateSystem::CalculateModelMatrix(GetEntity().GetTransform());
		const Vec3  viewPos     = camera.WorldToCameraSpace(modelMatrix.GetTranslation());
		const float linearDepth = -viewPos.z;

		drawCmdBuffer.Add(DrawCommand::Create(*mesh, modelMatrix, linearDepth, castShadows));
	}

} // namespace Engine::Components