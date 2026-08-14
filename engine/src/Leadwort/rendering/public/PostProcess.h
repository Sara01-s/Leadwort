#pragma once

#include <Leadwort/rendering/bindables/public/Material.h>
#include <Leadwort/rendering/bindables/public/Mesh.h>
#include <Leadwort/rendering/bindables/public/Shader.h>

#include <memory>

namespace Leadwort::Rendering {

	class PostProcess {
	public:
		explicit PostProcess(const Shared<Bindables::Shader>& shader);

		void Render(GpuID sceneTextureID) const;

	private:
		Shared<Bindables::Mesh> m_Mesh;
		Shared<Bindables::Material> m_Material;
	};

} // namespace Engine::Rendering