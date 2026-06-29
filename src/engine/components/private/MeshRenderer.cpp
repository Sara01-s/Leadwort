#include "engine/components/public/MeshRenderer.h"

#include "engine/components/public/Transform.h"
#include "engine/core/public/Entity.h"
#include "engine/rendering/public/DrawCommands.h"
#include "engine/rendering/public/MatrixUtils.h"
#include "engine/utils/public/Logger.h"

namespace Engine::Components {

using namespace Rendering;

void MeshRenderer::EmitDrawCommand(DrawCommandBuffer& drawCmdBuffer, const Camera& camera) const {
	CORE_ASSERT(&GetEntity().GetTransform(), "MeshRenderer::Render: Entity transform is null.");
	CORE_ASSERT(mesh, "MeshRenderer::Render: Mesh is null.");

	auto const& activeMaterial = mesh->GetMaterial();
	CORE_ASSERT(activeMaterial, "MeshRenderer::Render: Material is null.");

	const Mat4  modelMatrix = MatrixUtils::CalculateModelMatrix(GetEntity().GetTransform());
	const Vec3  viewPos     = camera.WorldToCameraSpace(modelMatrix.GetTranslation());
	const float linearDepth = -viewPos.z;

	drawCmdBuffer.Add(DrawCommand::Create(*mesh, modelMatrix, linearDepth));
}

} // namespace Engine::Components