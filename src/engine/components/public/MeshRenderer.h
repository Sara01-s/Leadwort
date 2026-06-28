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

	void SetMaterial(const Shared<Rendering::Bindables::Material>& material) {
		m_OverrideMaterial = material;
	}

	void ClearMaterial() {
		m_OverrideMaterial = nullptr;
	}

	Rendering::Bindables::Material* GetMaterial() const {
		return m_OverrideMaterial.get(); 
	}

	void Render(const Camera* camera) override;

private:
	Shared<Rendering::Bindables::Material> m_OverrideMaterial = nullptr;
};

} // namespace Engine::Components