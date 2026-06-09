#pragma once

#include "engine/components/public/Renderer.h"
#include "engine/rendering/bindables/public/Material.h"
#include "engine/rendering/bindables/public/Mesh.h"
#include "engine/systems/public/RenderSystem.h"

namespace Engine::Components { class Camera; }
namespace Engine::Components::Behaviours { class DirectionalLight; }

namespace Engine::Components {

class MeshRenderer : public Renderer {
public:
	Rendering::Bindables::Mesh* mesh = nullptr;

	void SetMaterial(const std::shared_ptr<Rendering::Bindables::Material>& material) const {
		this->mesh->SetMaterial(material);
	}

	void Render(const Camera* camera) override;
};

} // namespace Engine::Components