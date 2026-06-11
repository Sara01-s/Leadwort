#pragma once

#include <engine/rendering/bindables/public/Material.h>
#include <engine/rendering/bindables/public/CubeMap.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Engine::Rendering {

class Skybox {
public:
	Skybox();
	~Skybox();

	Skybox(const Skybox&) = delete;
	Skybox& operator=(const Skybox&) = delete;

	void Render() const;

private:
	unsigned int m_Vao = 0, m_Vbo = 0, m_Ibo = 0;

	Shared<Bindables::CubeMap> m_CubeMap = nullptr;
	Shared<Bindables::Material> m_SkyboxMaterial = nullptr;
};

}