#pragma once
#include "Leadwort/core/math/public/Vec3.h"

#include <vector>

namespace Leadwort {
	struct Mat4;
}

namespace Leadwort::Utils::GizmoShapes {

	struct DebugLine {
		Vec3 start{};
		Vec3 end{};
	};

	std::vector<DebugLine> Circle(const Vec3& center, const Vec3& normal, float radius, int segments = 32);
	std::vector<DebugLine> Sphere(const Vec3& center, float radius, int segments = 16);
	std::vector<DebugLine> Cone(const Vec3& apex, const Vec3& direction, float angle, float length, int segments = 16);
	std::vector<DebugLine> Box(const Mat4& worldMatrix, const Vec3& halfExtents);
	std::vector<DebugLine> Frustum(const Mat4& worldMatrix, float fovY, float aspect, float nearPlane, float farPlane);

}