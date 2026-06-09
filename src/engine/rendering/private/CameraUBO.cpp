#include "engine/rendering/public/CameraUBO.h"

#include "engine/components/public/Camera.h"
#include "engine/core/public/Entity.h"
#include "engine/components/public/Transform.h"
#include <glm/gtc/type_ptr.hpp>

namespace Engine::Rendering {

static constexpr GLuint CAMERA_UBO_BINDING = 0;

void CameraUBO::Init() {
	glGenBuffers(1, &m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	// view + proj + cameraPos = 2 * mat4 + vec4 = 144 bytes.
	glBufferData(GL_UNIFORM_BUFFER, 144, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING, m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

CameraUBO::~CameraUBO() {
	if (m_UBO) { glDeleteBuffers(1, &m_UBO); }
}

void CameraUBO::Update(const Components::Camera* camera) const {
	const glm::mat4 view = camera->GetViewMatrix();
	const glm::mat4 proj = camera->GetProjectionMatrix();
	const auto cameraPosition = glm::vec4(
		camera->entity->transform->GetWorldPosition(), 0.0f
	);

	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferSubData(GL_UNIFORM_BUFFER,   0, sizeof(glm::mat4), glm::value_ptr(view));
	glBufferSubData(GL_UNIFORM_BUFFER,  64, sizeof(glm::mat4), glm::value_ptr(proj));
	glBufferSubData(GL_UNIFORM_BUFFER, 128, sizeof(glm::vec4), glm::value_ptr(cameraPosition));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

} // namespace Engine::Rendering