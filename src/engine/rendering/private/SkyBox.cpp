#include "engine/rendering/public/Skybox.h"
#include "engine/asset-management/public/AssetManager.h"

namespace Engine::Rendering {

Skybox::Skybox() {
	const auto shader = AssetManagement::EngineAssets::GetShader("shaders/shd_skybox.glsl");
	m_SkyboxMaterial = AssetManagement::EngineAssets::CreateMaterial(shader);

	constexpr float vertices[] = {
		-1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 1.0f
	};

	constexpr int indices[] = {
		// -Z
		2, 1, 0,  3, 2, 0,
		// +Z
		5, 6, 4,  6, 7, 4,
		// -X
		7, 3, 0,  4, 7, 0,
		// +X
		6, 5, 1,  2, 6, 1,
		// -Y
		5, 4, 0,  1, 5, 0,
		// +Y
		6, 2, 3,  7, 6, 3,
	};

	const std::array<std::string, 6> cubeMapTexturePaths = {
		"textures/skyboxes/alt/alt_ft.jpg",
		"textures/skyboxes/alt/alt_bk.jpg",
		"textures/skyboxes/alt/alt_up.jpg",
		"textures/skyboxes/alt/alt_dn.jpg",
		"textures/skyboxes/alt/alt_rt.jpg",
		"textures/skyboxes/alt/alt_lf.jpg",
	};

	glGenVertexArrays(1, &m_Vao);
	glGenBuffers(1, &m_Vbo);
	glGenBuffers(1, &m_Ibo);

	glBindVertexArray(m_Vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

	glBindVertexArray(0);

	const auto cubeMap = AssetManagement::EngineAssets::GetCubeMap(cubeMapTexturePaths);
	m_SkyboxMaterial->SetCubeMap("_SkyboxTexture", cubeMap.get());
	m_CubeMap = cubeMap;
}

Skybox::~Skybox() {
	glDeleteBuffers(1, &m_Vbo);
	glDeleteBuffers(1, &m_Ibo);
	glDeleteVertexArrays(1, &m_Vao);
}

void Skybox::Render() const {
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);

	m_SkyboxMaterial->Bind();

	glBindVertexArray(m_Vao);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
}

}