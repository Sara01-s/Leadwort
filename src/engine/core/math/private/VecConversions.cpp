#define GLM_ENABLE_EXPERIMENTAL
#include "../public/Vec2.h"
#include "../public/Vec3.h"
#include "../public/Vec4.h"
#include "../public/Mat4.h"
#include "../public/Math.h"

namespace Engine {

// ── Vec2 conversions ──────────────────────────────────────────────────────
Vec3 Vec2::ToVec3(const float z) const { return Vec3(x, y, z); }
Vec4 Vec2::ToVec4(const float z, const float w) const { return Vec4(x, y, z, w); }

// ── Vec3 conversions ──────────────────────────────────────────────────────
Vec2 Vec3::ToVec2() const { return Vec2(x, y); }
Vec4 Vec3::ToVec4(const float w) const { return Vec4(x, y, z, w); }

// ── Vec3 swizzles ─────────────────────────────────────────────────────────
Vec2 Vec3::XY() const { return Vec2(x, y); }
Vec2 Vec3::XZ() const { return Vec2(x, z); }
Vec2 Vec3::YZ() const { return Vec2(y, z); }

// ── Vec4 conversions ──────────────────────────────────────────────────────
Vec2 Vec4::ToVec2() const { return Vec2(x, y); }
Vec3 Vec4::ToVec3() const { return Vec3(x, y, z); }

// ── Vec4 swizzles ─────────────────────────────────────────────────────────
Vec2 Vec4::XY() const { return Vec2(x, y); }
Vec2 Vec4::ZW() const { return Vec2(z, w); }
Vec3 Vec4::XYZ() const { return Vec3(x, y, z); }
Vec3 Vec4::XYW() const { return Vec3(x, y, w); }
Vec3 Vec4::XZW() const { return Vec3(x, z, w); }
Vec3 Vec4::YZW() const { return Vec3(y, z, w); }

// ── Quat conversions ──────────────────────────────────────────────────────
Quat Quat::FromMatrix(const Mat4& matrix) {
	return Quat(glm::quat_cast(matrix.GetInternalMatrix()));
}

Quat Quat::LookRotation(const Vec3& forward, const Vec3& up) {
	const float forwardLengthSq = LengthSquared(forward);
	if (forwardLengthSq < 1e-6f) {
		return Identity();
	}

	const Vec3 f = forward / Sqrt(forwardLengthSq);

	const Vec3 alternativeUp = LengthSquared(Cross(up, f)) < 0.0001f
	   ? (LengthSquared(Cross(Vec3::Up(), f)) < 0.0001f ? Vec3::Right() : Vec3::Up())
	   : up;

	const Vec3 r = Normalize(Cross(alternativeUp, f));
	const Vec3 u = Cross(f, r);

	glm::mat4 glmMatrix(1.0f);
	glmMatrix[0] = glm::vec4(glm::vec3(r), 0.0f); // Right
	glmMatrix[1] = glm::vec4(glm::vec3(u), 0.0f); // Up
	glmMatrix[2] = glm::vec4(glm::vec3(f), 0.0f); // Forward
	glmMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	return Quat(glm::quat_cast(glmMatrix));
}

}