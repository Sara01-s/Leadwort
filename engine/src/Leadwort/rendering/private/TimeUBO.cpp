#include <Leadwort/rendering/public/TimeUBO.h>
#include "Leadwort/core/math/public/Vec4.h"
#include <Leadwort/rendering/public/rendergraph/GlobalSlots.h>

namespace Leadwort::Rendering {

void TimeUBO::Initialize() {
	glGenBuffers(1, &m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Vec4), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, UBOSlots::TimeDataBinding, m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

TimeUBO::~TimeUBO() {
	if (m_UBO != 0) {
		glDeleteBuffers(1, &m_UBO);
		m_UBO = 0;
	}
}

void TimeUBO::Update(const float time, const float deltaTime) const {
	const Vec4 timeVec { time, time * 0.5f, time * 2.0f, deltaTime };

	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Vec4), &timeVec);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

} // namespace Engine::Rendering