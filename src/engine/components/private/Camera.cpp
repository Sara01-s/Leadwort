#include "engine/components/public/Camera.h"

#include "engine/core/public/Entity.h"
#include "engine/rendering/public/MatrixUtils.h"

namespace Engine::Components {

Vec3 Camera::WorldToCameraSpace(const Vec3& worldPosition) const noexcept {
	return (GetViewMatrix() * worldPosition.ToVec4(1.0f)).ToVec3();
}

bool Camera::ShouldRender(const Core::Entity& entity) const {
	return (entity.layerMask & cullingMask) != 0;
}

Mat4 Camera::GetViewMatrix() const {
	return Rendering::MatrixUtils::CalculateViewMatrix(GetEntity().GetTransform());
}

Mat4 Camera::GetProjectionMatrix() const {
	return Rendering::MatrixUtils::CalculateProjectionMatrix({fov, nearPlane, farPlane, aspect});
}

} // namespace Engine::Components