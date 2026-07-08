#pragma once

#include "asset-management/public/DefaultAssets.h"
#include "components/public/Renderer.h"
#include "rendering/bindables/public/Material.h"
#include "rendering/bindables/public/Mesh.h"
#include "systems/public/RenderSystem.h"
#include "utils/public/PrimitiveMeshes.h"

namespace Engine::Components { class Camera; }
namespace Engine::Components::Behaviours { class DirectionalLight; }

namespace Engine::Components {

class MeshRenderer : public Renderer {
public:
	Shared<Rendering::Bindables::Mesh> mesh = Utils::PrimitiveMeshes::Get().Cube();

public:
	void EmitDrawCommand(Rendering::DrawCommandBuffer& drawCmdBuffer, const Camera& camera) const override;
	[[nodiscard]] std::optional<AABB> GetAABB() const override { return mesh->GetAABB(); }
};

} // namespace Engine::Components