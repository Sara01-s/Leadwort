#pragma once

#include <Leadwort/rendering/bindables/public/Material.h>
#include <Leadwort/rendering/bindables/public/CubeMap.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Leadwort::Rendering {

class Skybox {
public:
	Skybox();
	~Skybox();

	Skybox(const Skybox&) = delete;
	Skybox& operator=(const Skybox&) = delete;

	void Render() const;

private:
	GpuID m_Vao { 0 };
	GpuID m_Vbo { 0 };
	GpuID m_Ibo { 0 };

	Shared<Bindables::CubeMap> m_CubeMap{};
	Shared<Bindables::Material> m_SkyboxMaterial{};
};

}