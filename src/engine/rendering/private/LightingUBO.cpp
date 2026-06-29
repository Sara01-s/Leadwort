#include "engine/rendering/public/LightingUBO.h"

#include "engine/components/behaviours/public/DirectionaLight.h"
#include "engine/core/public/Entity.h"
#include "engine/components/public/Transform.h"

namespace Engine::Rendering {

static constexpr GLuint LIGHTING_UBO_BINDING   = 1;
static constexpr GLuint LIGHTING_UBO_SIZE_BYTES = 32;

// std140 layout:
//   offset  0 → vec4 direction  (vec3 + 4 bytes padding)
//   offset 16 → vec3 color      (12 bytes)
//   offset 28 → float intensity (4 bytes, packed into color's padding slot)

void LightingUBO::Initialize() {
	glGenBuffers(1, &m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferData(GL_UNIFORM_BUFFER, LIGHTING_UBO_SIZE_BYTES, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, LIGHTING_UBO_BINDING, m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

LightingUBO::~LightingUBO() {
	if (m_UBO) {
		glDeleteBuffers(1, &m_UBO);
	}
}

void LightingUBO::Update(const Components::Behaviours::DirectionalLight* light) const {
	if (!light) {
		// Upload zeroed data so shaders don't read garbage
		constexpr float zeroes[LIGHTING_UBO_SIZE_BYTES / sizeof(float)] = {};
		glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, LIGHTING_UBO_SIZE_BYTES, zeroes);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		return;
	}

	const Vec4 direction  = light->GetEntity().GetTransform().GetForward().ToVec4();
	const auto color      = Vec3(light->color.r, light->color.g, light->color.b);
	const float intensity = light->intensity;

	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferSubData(GL_UNIFORM_BUFFER,  0, sizeof(Vec4),  direction.ToPtr());
	glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(Vec3),  &color.x);
	glBufferSubData(GL_UNIFORM_BUFFER, 28, sizeof(float), &intensity);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

} // namespace Engine::Rendering