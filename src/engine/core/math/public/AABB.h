#pragma once

#include "engine/core/math/public/Math.h"

namespace Engine {

struct AABB {
	Vec3 min { +FLT_MAX, +FLT_MAX, +FLT_MAX };
	Vec3 max { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	void Expand(const Vec3& point) noexcept {
		min.x = Min(min.x, point.x);
		min.y = Min(min.y, point.y);
		min.z = Min(min.z, point.z);
		max.x = Max(max.x, point.x);
		max.y = Max(max.y, point.y);
		max.z = Max(max.z, point.z);
	}

	[[nodiscard]] constexpr Vec3 Center() const noexcept { return (min + max) * 0.5f; }
	[[nodiscard]] constexpr Vec3 Extents() const noexcept { return (max - min) * 0.5f; }
	[[nodiscard]] constexpr bool IsValid() const noexcept { return min.x <= max.x; }

	// Returns an AABB in world space by transforming all 8 corners.
	[[nodiscard]] AABB Transformed(const Mat4& matrix) const noexcept {
		const std::array corners {
			Vec3(min.x, min.y, min.z),
			Vec3(max.x, min.y, min.z),
			Vec3(min.x, max.y, min.z),
			Vec3(max.x, max.y, min.z),
			Vec3(min.x, min.y, max.z),
			Vec3(max.x, min.y, max.z),
			Vec3(min.x, max.y, max.z),
			Vec3(max.x, max.y, max.z),
		};

		AABB result{};

		for (const auto& corner : corners) {
			result.Expand((matrix * corner.ToVec4(1.0f)).ToVec3());
		}

		return result;
	}
};

} // namespace Engine::Rendering