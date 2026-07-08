#pragma once
#include "Math.h"
#include "Vec3.h"

namespace Engine {
	// Plane: ax + by + cz + d = 0
	// normal = (a, b, c), d = d
	struct Plane {
		Plane() = delete;
		explicit Plane(const Vec3& normal, const float scale) : normal(normal), scale(scale) {}

		Vec3 normal { 0.0f, 1.0f, 0.0f };
		float scale { 0.0f };

		// Signed distance from point to plane.
		// Positive = in front, negative = behind.
		[[nodiscard]]
		float SignedDistance(const Vec3& point) const {
			return Dot(normal, point) + scale;
		}
	};
}
