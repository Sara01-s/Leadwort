#include <Leadwort/components/public/Camera.h>
#include <Leadwort/core/public/Entity.h>
#include <Leadwort/rendering/public/CoordinateSystem.h>

namespace Leadwort::Components {

Vec3 Camera::WorldToCameraSpace(const Vec3& worldPosition) const noexcept {
	return (GetViewMatrix() * worldPosition.ToVec4(1.0f)).ToVec3();
}

bool Camera::ShouldRender(const Core::Entity& entity) const noexcept {
	return (entity.layerMask & cullingMask) != 0;
}

Mat4 Camera::GetViewMatrix() const noexcept {
	return Rendering::CoordinateSystem::CalculateViewMatrix(GetEntity().GetTransform());
}

Mat4 Camera::GetProjectionMatrix() const noexcept {
	return Rendering::CoordinateSystem::CalculateProjectionMatrix({fov, nearPlane, farPlane, aspect});
}

Ray Camera::ScreenPointToRay(const Vec2& normalizedScreenPoint) const noexcept {
	const float ndcX {  normalizedScreenPoint.x * 2.0f - 1.0f };
	const float ndcY { -(normalizedScreenPoint.y * 2.0f - 1.0f) };

	const Mat4 invProj { Inverse(GetProjectionMatrix()) };
	const Mat4 invView { Inverse(GetViewMatrix()) };

	const Vec4 nearClip { ndcX, ndcY, -1.0f, 1.0f };
	const Vec4 farClip  { ndcX, ndcY,  1.0f, 1.0f };

	Vec4 nearView { invProj * nearClip };
	Vec4 farView  { invProj * farClip };
	nearView /= nearView.w;
	farView  /= farView.w;

	const Vec3 nearWorld { (invView * nearView).ToVec3() };
	const Vec3 farWorld  { (invView * farView).ToVec3() };
	const Vec3 direction { (farWorld - nearWorld).Normalized() };

	return Ray { nearWorld, direction };
}

} // namespace Engine::Components