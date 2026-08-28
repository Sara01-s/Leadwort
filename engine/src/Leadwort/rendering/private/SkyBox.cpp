#include <Leadwort/asset-management/public/AssetDatabase.h>
#include <Leadwort/rendering/public/Skybox.h>

namespace Leadwort::Rendering {

	Skybox::Skybox(const std::string_view exrPath) : m_ExrPath(exrPath) {
		const auto shader = AssetManagement::EngineAssets::GetShader("shaders/shd_skybox.glsl");
		m_SkyboxMaterial = AssetManagement::EngineAssets::CreateMaterial(shader);

		constexpr float vertices[] {
			-1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
			-1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 1.0f
		};

		constexpr int indices[] {
			2, 1, 0,  3, 2, 0,
			5, 6, 4,  6, 7, 4,
			7, 3, 0,  4, 7, 0,
			6, 5, 1,  2, 6, 1,
			5, 4, 0,  1, 5, 0,
			6, 2, 3,  7, 6, 3,
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

		const auto& exrTexture { AssetManagement::EngineAssets::GetTexture(exrPath) };
		m_SkyboxMaterial->SetTexture("_SkyboxTexture", exrTexture);

		m_SkyboxMaterial->SetFloat("_RotationDegrees", m_RotationDegrees);
		m_SkyboxMaterial->SetVec3("_Tint", m_Tint);
		m_SkyboxMaterial->SetFloat("_Exposure", m_Exposure);
	}

	Skybox::~Skybox() {
		glDeleteBuffers(1, &m_Vbo);
		glDeleteBuffers(1, &m_Ibo);
		glDeleteVertexArrays(1, &m_Vao);
	}

	void Skybox::Render() const {
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		glDisable(GL_CULL_FACE);

		m_SkyboxMaterial->Bind();

		glBindVertexArray(m_Vao);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
	}

	void Skybox::SetRotation(const float degrees) noexcept {
		m_RotationDegrees = degrees;
		m_SkyboxMaterial->SetFloat("_RotationDegrees", degrees);
	}

	void Skybox::SetTint(const Vec3& tint) noexcept {
		m_Tint = tint;
		m_SkyboxMaterial->SetVec3("_Tint", tint);
	}

	void Skybox::SetExposure(const float exposure) noexcept {
		m_Exposure = exposure;
		m_SkyboxMaterial->SetFloat("_Exposure", exposure);
	}

}