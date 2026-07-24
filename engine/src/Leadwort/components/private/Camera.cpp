#include <Leadwort/components/public/Camera.h>
#include <Leadwort/core/public/Entity.h>
#include <Leadwort/rendering/public/CoordinateSystem.h>

namespace Leadwort::Components {

Vec3 Camera::WorldToCameraSpace(const Vec3& worldPosition) const noexcept {
	return (GetViewMatrix() * worldPosition.ToVec4(1.0f)).ToVec3();
}

bool Camera::ShouldRender(const Core::Entity& entity) const {
	return (entity.layerMask & cullingMask) != 0;
}

Mat4 Camera::GetViewMatrix() const {
	return Rendering::CoordinateSystem::CalculateViewMatrix(GetEntity().GetTransform());
}

Mat4 Camera::GetProjectionMatrix() const {
	return Rendering::CoordinateSystem::CalculateProjectionMatrix({fov, nearPlane, farPlane, aspect});
}

} // namespace Engine::Components