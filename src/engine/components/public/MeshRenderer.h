#pragma once

#include "engine/asset-management/public/DefaultAssets.h"
#include "engine/components/public/Renderer.h"
#include "engine/rendering/bindables/public/Material.h"
#include "engine/rendering/bindables/public/Mesh.h"
#include "engine/systems/public/RenderSystem.h"
#include "engine/utils/public/PrimitiveMeshes.h"

namespace Engine::Components { class Camera; }
namespace Engine::Components::Behaviours { class DirectionalLight; }

namespace Engine::Components {

class MeshRenderer : public Renderer {
public:
	Shared<Rendering::Bindables::Mesh> mesh = Utils::PrimitiveMeshes::Get().Cube();

public:
	void EmitDrawCommand(Rendering::DrawCommandBuffer& drawCmdBuffer, const Camera& camera) const override;
};

} // namespace Engine::Components