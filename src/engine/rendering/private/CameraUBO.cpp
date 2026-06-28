#include "engine/rendering/public/CameraUBO.h"

#include "engine/components/public/Camera.h"
#include "engine/core/public/Entity.h"
#include "engine/components/public/Transform.h"

namespace Engine::Rendering {

static constexpr GLuint CAMERA_UBO_BINDING = 0;
static constexpr GLuint CAMERA_UBO_SIZE_BYTES = 144; // view + proj + cameraPos = 2 * mat4 + vec4 = 144 bytes.

void CameraUBO::Init() {
	glGenBuffers(1, &m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferData(GL_UNIFORM_BUFFER, CAMERA_UBO_SIZE_BYTES, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING, m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

CameraUBO::~CameraUBO() {
	if (m_UBO) {
		glDeleteBuffers(1, &m_UBO);
	}
}

void CameraUBO::Update(const Components::Camera& camera) const {
	const Mat4 view = camera.GetViewMatrix();
	const Mat4 proj = camera.GetProjectionMatrix();
	const auto cameraPosition = camera.GetEntity().GetTransform().GetWorldPosition().ToVec4();

	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferSubData(GL_UNIFORM_BUFFER,   0, sizeof(Mat4), view.ToPtr());
	glBufferSubData(GL_UNIFORM_BUFFER,  64, sizeof(Mat4), proj.ToPtr());
	glBufferSubData(GL_UNIFORM_BUFFER, 128, sizeof(Vec4), cameraPosition.ToPtr());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

} // namespace Engine::Rendering