#pragma once

#include "engine/rendering/bindables/public/Material.h"
#include "engine/rendering/bindables/public/Mesh.h"
#include "engine/rendering/bindables/public/Shader.h"

#include <memory>

namespace Engine::Rendering {

class PostProcess {
public:
	explicit PostProcess(const std::shared_ptr<Bindables::Shader>& shader);

	void Render(unsigned int sceneTextureID, const float width, const float height);

private:
	std::shared_ptr<Bindables::Mesh> m_Mesh;
	Bindables::Material m_Material;
};

} // namespace Engine::Rendering