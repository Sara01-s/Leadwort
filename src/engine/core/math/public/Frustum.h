#pragma once

#include "AABB.h"
#include "Plane.h"
#include "Mat4.h"
#include "Vec3.h"
#include "Vec4.h"

#undef near
#undef far

namespace Engine {

struct Frustum {
	explicit Frustum(const std::array<Plane, 6>& planes) noexcept : planes(planes) {}

    std::array<Plane, 6> planes; // Left, Right, Bottom, Top, Near, Far

    // Extract frustum planes from a view-projection matrix.
    // Uses Gribb/Hartmann method, works for GL clip space.
    static Frustum FromViewProjection(const Mat4& viewProjection) noexcept {
    	const Mat4& matrix = viewProjection.Transposed();

        const Vec4 c0 = matrix.GetColumn(0);
        const Vec4 c1 = matrix.GetColumn(1);
        const Vec4 c2 = matrix.GetColumn(2);
        const Vec4 c3 = matrix.GetColumn(3);

        // Each plane is a row combination of the VP matrix
        const Vec4 left   = c3 + c0;
        const Vec4 right  = c3 - c0;
        const Vec4 bottom = c3 + c1;
        const Vec4 top    = c3 - c1;
        const Vec4 near   = c3 + c2;
        const Vec4 far    = c3 - c2;

        auto makePlane = [](const Vec4& v) -> Plane {
            const float len = Vec3(v.x, v.y, v.z).Length();
            return Plane(Vec3(v.x, v.y, v.z) / len, v.w / len);
        };

    	const std::array planes {
    		makePlane(left),
    		makePlane(right),
    		makePlane(bottom),
    		makePlane(top),
    		makePlane(near),
    		makePlane(far)
    	};

		return Frustum(planes);
    }

    // Test an AABB against the frustum.
    // Uses the "positive vertex" method, fast and conservative.
    // Returns false if the AABB is fully outside any plane.
    [[nodiscard]] bool Intersects(const AABB& aabb) const noexcept {
        for (const auto& plane : planes) {
            // Positive vertex: the AABB corner furthest in the plane's normal direction
            const Vec3 positive {
                plane.normal.x >= 0.0f ? aabb.max.x : aabb.min.x,
                plane.normal.y >= 0.0f ? aabb.max.y : aabb.min.y,
                plane.normal.z >= 0.0f ? aabb.max.z : aabb.min.z,
            };

            // If the positive vertex is behind the plane, the whole AABB is outside
            if (plane.SignedDistance(positive) < 0.0f) {
                return false;
            }
        }

        return true;
    }
};

} // namespace Engine::Rendering