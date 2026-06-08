#pragma once

#include "engine/components/public/Transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tuple>

namespace Engine::Rendering::MatrixUtils {

// LH (Y-up, +Z forward) → RH conversion applied at a single point
inline constexpr auto LH_TO_RH = glm::mat4(
	1,  0,  0,  0,
	0,  1,  0,  0,
	0,  0, -1,  0,
	0,  0,  0,  1
);

inline glm::mat4 CalculateModelMatrix(const Components::Transform& transform) {
	return LH_TO_RH * transform.GetWorldMatrix();
}

inline glm::mat4 CalculateViewMatrix(const Components::Transform& cameraTransform) {
	return LH_TO_RH * glm::inverse(cameraTransform.GetWorldMatrix());
}

inline glm::mat4 CalculateProjectionMatrix(const float fovY, const float nearPlane, const float farPlane, const float aspect) {
	return glm::perspective(glm::radians(fovY), aspect, nearPlane, farPlane);
}

inline glm::mat4 CalculateMvpMatrix(
	const glm::mat4& model,
	const glm::mat4& view,
	const glm::mat4& projection
) {
	return projection * view * model;
}

inline std::tuple<glm::mat4, glm::mat4, glm::mat4> CalculateMvpMatrix(
	const Components::Transform& transform,
	const Components::Transform& cameraTransform,
	const float fovY,
	const float nearPlane,
	const float farPlane,
	const float aspect
) {
	const auto model = CalculateModelMatrix(transform);
	const auto view  = CalculateViewMatrix(cameraTransform);
	const auto proj  = CalculateProjectionMatrix(fovY, nearPlane, farPlane, aspect);

	return { model, view, proj };
}

} // namespace Engine::Rendering::MatrixUtils