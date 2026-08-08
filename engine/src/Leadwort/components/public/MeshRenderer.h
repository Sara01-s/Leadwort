#pragma once

#include <Leadwort/asset-management/public/DefaultAssets.h>
#include <Leadwort/components/public/Renderer.h>
#include <Leadwort/rendering/bindables/public/Mesh.h>
#include <Leadwort/systems/public/RenderSystem.h>
#include <Leadwort/utils/public/PrimitiveMeshes.h>

namespace Leadwort::Components { class Camera; }
namespace Leadwort::Components::Behaviours { class DirectionalLight; }

namespace Leadwort::Components {

class MeshRenderer : public Renderer {
public:
	using Mesh = Rendering::Bindables::Mesh;

	LW_REFLECT(MeshRenderer,
		LW_FIELD(AssetRef, mesh, "Mesh")
	)
public:
	Shared<Mesh> mesh { Utils::PrimitiveMeshes::Get().Cube() };

public:
	void EmitDrawCommand(Rendering::DrawCommandBuffer& drawCmdBuffer, const Camera& camera) const override;
	[[nodiscard]] std::optional<AABB> GetAABB() const override { return mesh->GetAABB(); }
};

} // namespace Engine::Components