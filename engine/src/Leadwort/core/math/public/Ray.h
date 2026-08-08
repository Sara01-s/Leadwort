#pragma once

#include <Leadwort/core/math/public/Math.h>
#include <Leadwort/core/math/public/AABB.h>

#include <optional>

namespace Leadwort {

struct Ray {
	Vec3 origin{};
	Vec3 direction{}; // normalized assumed.

	[[nodiscard]] Vec3 At(const float t) const noexcept {
		return origin + direction * t;
	}

	[[nodiscard]] std::optional<float> IntersectsAABB(const AABB& box) const noexcept {
		float tMin { 0.0f };
		float tMax { FLT_MAX };

		const float originArr[3] { origin.x, origin.y, origin.z };
		const float dirArr[3]    { direction.x, direction.y, direction.z };
		const float boxMinArr[3] { box.min.x, box.min.y, box.min.z };
		const float boxMaxArr[3] { box.max.x, box.max.y, box.max.z };

		for (int axis = 0; axis < 3; ++axis) {
			if (std::abs(dirArr[axis]) < 1e-8f) {
				if (originArr[axis] < boxMinArr[axis] || originArr[axis] > boxMaxArr[axis]) {
					return std::nullopt;
				}
				continue;
			}

			const float invDir { 1.0f / dirArr[axis] };
			float t1 { (boxMinArr[axis] - originArr[axis]) * invDir };
			float t2 { (boxMaxArr[axis] - originArr[axis]) * invDir };

			if (t1 > t2) {
				std::swap(t1, t2);
			}

			tMin = Max(tMin, t1);
			tMax = Min(tMax, t2);

			if (tMin > tMax) {
				return std::nullopt;
			}
		}

		return tMin;
	}
};

} // namespace Leadwort