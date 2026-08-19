#include <Leadwort/rendering/public/CameraUBO.h>

#include "Leadwort/rendering/public/rendergraph/GlobalSlots.h"

#include <Leadwort/components/public/Camera.h>
#include <Leadwort/components/public/Transform.h>
#include <Leadwort/core/public/Entity.h>

namespace Leadwort::Rendering {

	void CameraUBO::Initialize() {
		static_assert(sizeof(CameraDataGPU) == 160);

		glGenBuffers(1, &m_UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraDataGPU), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, UBOSlots::CameraDataBinding, m_UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	CameraUBO::~CameraUBO() {
		if (m_UBO) {
			glDeleteBuffers(1, &m_UBO);
		}
	}

	void CameraUBO::Update(const Components::Camera& camera) const {
		const CameraDataGPU data {
			.view = camera.GetViewMatrix(),
			.projection = camera.GetProjectionMatrix(),
			.cameraPosition = camera.GetEntity().GetTransform().GetWorldPosition().ToVec4(),
			.padding = Vec4()
		};

		glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraDataGPU), &data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

} // namespace Engine::Rendering