#pragma once

#include "rendering/bindables/public/Material.h"
#include "rendering/bindables/public/Mesh.h"
#include "rendering/bindables/public/Shader.h"

#include <memory>

namespace Engine::Rendering {

class PostProcess {
public:
	explicit PostProcess(const Shared<Bindables::Shader>& shader);

	void Render(GpuID sceneTextureID) const;

private:
	Shared<Bindables::Mesh> m_Mesh;
	Shared<Bindables::Material> m_Material;
};

} // namespace Engine::Rendering