#pragma once

#include "engine/components/public/Camera.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Engine::Rendering {

// std140 layout — binding = 0
// offset   0 : mat4  view          (64 bytes)
// offset  64 : mat4  projection    (64 bytes)
// offset 128 : vec4  cameraPos     (16 bytes) vec3 padded to vec4
// total      : 144 bytes

class CameraUBO {
public:
	CameraUBO()  = default;
	~CameraUBO() { Destroy(); }

	CameraUBO(const CameraUBO&)            = delete;
	CameraUBO& operator=(const CameraUBO&) = delete;
	CameraUBO(CameraUBO&&)                 = delete;
	CameraUBO& operator=(CameraUBO&&)      = delete;

	void Init() {
		glGenBuffers(1, &m_Id);
		glBindBuffer(GL_UNIFORM_BUFFER, m_Id);
		glBufferData(GL_UNIFORM_BUFFER, SIZE, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, BINDING, m_Id);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void Update(const Components::Camera* camera) const {
		const glm::mat4 view = camera->GetViewMatrix();
		const glm::mat4 proj = camera->GetProjectionMatrix();
		const auto cameraPosition = glm::vec4(camera->entity->transform->GetWorldPosition(), 0.0f);

		glBindBuffer(GL_UNIFORM_BUFFER, m_Id);
		glBufferSubData(GL_UNIFORM_BUFFER,   0, 64, &view[0][0]);
		glBufferSubData(GL_UNIFORM_BUFFER,  64, 64, &proj[0][0]);
		glBufferSubData(GL_UNIFORM_BUFFER, 128, 16, &cameraPosition[0]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void Destroy() {
		if (m_Id != 0) {
			glDeleteBuffers(1, &m_Id);
			m_Id = 0;
		}
	}

	[[nodiscard]] GLuint GetId() const { return m_Id; }

private:
	static constexpr GLuint BINDING = 0;
	static constexpr GLsizeiptr SIZE = 144;

	GLuint m_Id = 0;
};

} // namespace Engine::Rendering