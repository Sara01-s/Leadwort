// GizmoShapes.cpp
#include "../public/GizmoShapes.h"

#include <Leadwort/core/math/public/Math.h>
#include <cmath>

namespace Leadwort::Utils::GizmoShapes {

    static void GetOrthonormalBasis(const Vec3& normal, Vec3& outTangent, Vec3& outBitangent) {
        const Vec3 reference { (std::abs(Dot(normal, Vec3::Up())) > 0.99f) ? Vec3::Right() : Vec3::Up() };

        outTangent = Cross(normal, reference).Normalized();
        outBitangent = Cross(normal, outTangent).Normalized();
    }

    std::vector<DebugLine> Circle(const Vec3& center, const Vec3& normal, const float radius, const int segments) {
        std::vector<DebugLine> lines{};
        lines.reserve(segments);

        Vec3 tangent{}, bitangent{};
        GetOrthonormalBasis(normal.Normalized(), tangent, bitangent);

        Vec3 prevPoint { center + tangent * radius };

        for (int i = 1; i <= segments; i++) {
            const float t { (static_cast<float>(i) / static_cast<float>(segments)) * TAU };
            const Vec3 point { center + (tangent * std::cos(t) + bitangent * std::sin(t)) * radius };

            lines.push_back({ prevPoint, point });
            prevPoint = point;
        }

        return lines;
    }

    std::vector<DebugLine> Sphere(const Vec3& center, const float radius, const int segments) {
        std::vector<DebugLine> lines{};

        auto ringX { Circle(center, Vec3::Right(),   radius, segments) };
        auto ringY { Circle(center, Vec3::Up(),      radius, segments) };
        auto ringZ { Circle(center, Vec3::Forward(), radius, segments) };

        lines.reserve(ringX.size() + ringY.size() + ringZ.size());
        lines.insert(lines.end(), ringX.begin(), ringX.end());
        lines.insert(lines.end(), ringY.begin(), ringY.end());
        lines.insert(lines.end(), ringZ.begin(), ringZ.end());

        return lines;
    }

    std::vector<DebugLine> Cone(const Vec3& apex, const Vec3& direction, const float angle, const float length, const int segments) {
        std::vector<DebugLine> lines{};

        const Vec3 dir { direction.Normalized() };
        const float baseRadius { std::tan(angle) * length };
        const Vec3 baseCenter { apex + dir * length };

        auto baseCircle { Circle(baseCenter, dir, baseRadius, segments) };
        lines.reserve(baseCircle.size() + 4);
        lines.insert(lines.end(), baseCircle.begin(), baseCircle.end());

        Vec3 tangent{}, bitangent{};
        GetOrthonormalBasis(dir, tangent, bitangent);

        lines.push_back({ apex, baseCenter + tangent * baseRadius });
        lines.push_back({ apex, baseCenter - tangent * baseRadius });
        lines.push_back({ apex, baseCenter + bitangent * baseRadius });
        lines.push_back({ apex, baseCenter - bitangent * baseRadius });

        return lines;
    }

    std::vector<DebugLine> Box(const Mat4& worldMatrix, const Vec3& halfExtents) {
        std::vector<DebugLine> lines{};
        lines.reserve(12);

        const Vec3 localCorners[8] {
            Vec3(-halfExtents.x, -halfExtents.y, -halfExtents.z),
            Vec3( halfExtents.x, -halfExtents.y, -halfExtents.z),
            Vec3( halfExtents.x,  halfExtents.y, -halfExtents.z),
            Vec3(-halfExtents.x,  halfExtents.y, -halfExtents.z),
            Vec3(-halfExtents.x, -halfExtents.y,  halfExtents.z),
            Vec3( halfExtents.x, -halfExtents.y,  halfExtents.z),
            Vec3( halfExtents.x,  halfExtents.y,  halfExtents.z),
            Vec3(-halfExtents.x,  halfExtents.y,  halfExtents.z),
        };

        Vec3 worldCorners[8];
        for (int i = 0; i < 8; i++) {
            worldCorners[i] = (worldMatrix * Vec4(localCorners[i].x, localCorners[i].y, localCorners[i].z, 1.0f)).XYZ();
        }

        for (int i = 0; i < 4; i++) {
            lines.push_back({ worldCorners[i], worldCorners[(i + 1) % 4] });
        }

        for (int i = 0; i < 4; i++) {
            lines.push_back({ worldCorners[4 + i], worldCorners[4 + (i + 1) % 4] });
        }

        for (int i = 0; i < 4; i++) {
            lines.push_back({ worldCorners[i], worldCorners[4 + i] });
        }

        return lines;
    }

    std::vector<DebugLine> Frustum(const Mat4& worldMatrix, const float fovY, const float aspect, const float nearPlane, const float farPlane) {
        std::vector<DebugLine> lines{};
        lines.reserve(12);

        const float halfVFov { (fovY * 0.5f) * DegToRad };
        const float tanHalfVFov { std::tan(halfVFov) };

        const float nearHeight { tanHalfVFov * nearPlane };
        const float nearWidth  { nearHeight * aspect };
        const float farHeight  { tanHalfVFov * farPlane };
        const float farWidth   { farHeight * aspect };

    	const Vec3 localCorners[8] {
    		Vec3(-nearWidth,  nearHeight, nearPlane),
			Vec3( nearWidth,  nearHeight, nearPlane),
			Vec3( nearWidth, -nearHeight, nearPlane),
			Vec3(-nearWidth, -nearHeight, nearPlane),
			Vec3(-farWidth,  farHeight, farPlane),
			Vec3( farWidth,  farHeight, farPlane),
			Vec3( farWidth, -farHeight, farPlane),
			Vec3(-farWidth, -farHeight, farPlane),
		};

        Vec3 worldCorners[8];
        for (int i = 0; i < 8; i++) {
            worldCorners[i] = (worldMatrix * Vec4(localCorners[i].x, localCorners[i].y, localCorners[i].z, 1.0f)).XYZ();
        }

        for (int i = 0; i < 4; i++) {
            lines.push_back({ worldCorners[i], worldCorners[(i + 1) % 4] });                  // near rect
            lines.push_back({ worldCorners[4 + i], worldCorners[4 + (i + 1) % 4] });          // far rect
            lines.push_back({ worldCorners[i], worldCorners[4 + i] });                        // connectors
        }

        return lines;
    }

} // namespace Leadwort::Utils::GizmoShapes