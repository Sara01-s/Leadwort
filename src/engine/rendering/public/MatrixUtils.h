#pragma once

#include "engine/components/public/Transform.h"

#include <tuple>

namespace Engine::Rendering::MatrixUtils {

struct ProjectionSettings {
	float fovY;
	float nearPlane;
	float farPlane;
	float aspect;
};

// LH (Y-up, +Z forward) → RH conversion applied at a single point
inline constexpr auto LH_TO_RH_CORRECTION = Mat4(
	1.0f,  0.0f,  0.0f,  0.0f,
	0.0f,  1.0f,  0.0f,  0.0f,
	0.0f,  0.0f, -1.0f,  0.0f, // Invert Z
	0.0f,  0.0f,  0.0f,  1.0f
);

inline Mat4 CalculateModelMatrix(const Components::Transform& transform) {
	return transform.GetWorldMatrix();
}

inline Mat4 CalculateViewMatrix(const Components::Transform& cameraTransform) {
	const Mat4 world = cameraTransform.GetWorldMatrix();
	const Mat4 view = Inverse(world);
	return LH_TO_RH_CORRECTION * view;
}

inline Mat4 CalculateProjectionMatrix(const ProjectionSettings& settings) {
	return Mat4::PerspectiveRH(settings.fovY, settings.nearPlane, settings.farPlane, settings.aspect);
}

inline std::tuple<Mat4, Mat4, Mat4> GetMvpMatrices(
	const Components::Transform& transform,
	const Components::Transform& cameraTransform,
	const ProjectionSettings& settings
) {
	const auto model = CalculateModelMatrix(transform);
	const auto view  = CalculateViewMatrix(cameraTransform);
	const auto proj  = CalculateProjectionMatrix(settings);

	return { model, view, proj };
}

} // namespace Engine::Rendering::MatrixUtils