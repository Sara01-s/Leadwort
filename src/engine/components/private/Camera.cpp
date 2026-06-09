#include "engine/components/public/Camera.h"

#include "engine/core/public/Entity.h"
#include "engine/rendering/public/MatrixUtils.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Engine::Components {

bool Camera::ShouldRender(const Core::Entity* entity) const {
	return (entity->layerMask & cullingMask) != 0;
}

glm::mat4 Camera::GetViewMatrix() const {
	return Rendering::MatrixUtils::CalculateViewMatrix(*entity->transform);
}

glm::mat4 Camera::GetProjectionMatrix() const {
	return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}

} // namespace Engine::Components