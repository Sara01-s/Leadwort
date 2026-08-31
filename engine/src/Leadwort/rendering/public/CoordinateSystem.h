#pragma once

#include <Leadwort/components/public/Transform.h>

#include <tuple>

namespace Leadwort::Rendering::CoordinateSystem {

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

	// ── Gizmo camera basis ───────────────────────────────────────────────────
	// Third-party gizmo code (ImGuizmo) reads the camera basis out of the view matrix and
	// expects a right-handed one, where the third column points back at the viewer. That is
	// how it picks the near half of a rotation ring. CalculateViewMatrix folds the Z flip
	// into the view, which leaves that basis mirrored, and the gizmo then draws the far half
	// of every ring while its hit test keeps answering on the near half.
	//
	// Mirroring X between the view and the projection repairs the basis and nothing else:
	// the two mirrors cancel in the composed view-projection, so the gizmo still projects
	// exactly where it did. Crucially the world itself stays untouched — a gizmo handed a
	// mirrored world matrix would reverse every rotation it applies in an entity's own
	// space, since a mirror turns R(axis, angle) into R(axis, -angle).
	inline constexpr auto GIZMO_BASIS_CORRECTION = Mat4(
		-1.0f, 0.0f, 0.0f, 0.0f, // Mirror X
		 0.0f, 1.0f, 0.0f, 0.0f,
		 0.0f, 0.0f, 1.0f, 0.0f,
		 0.0f, 0.0f, 0.0f, 1.0f
	);

	inline Mat4 GizmoViewMatrix(const Mat4& viewMatrix) {
		return GIZMO_BASIS_CORRECTION * viewMatrix;
	}

	inline Mat4 GizmoProjectionMatrix(const Mat4& projectionMatrix) {
		return projectionMatrix * GIZMO_BASIS_CORRECTION;
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

} // namespace Engine::Rendering::CoordinateSystem