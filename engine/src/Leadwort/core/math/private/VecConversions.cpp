#define GLM_ENABLE_EXPERIMENTAL
#include "../public/Mat4.h"
#include "../public/Math.h"
#include "../public/Vec2.h"
#include "../public/Vec3.h"
#include "../public/Vec4.h"

namespace Leadwort {

	Vec3 Vec2::ToVec3(const float z) const { return Vec3(x, y, z); }
	Vec4 Vec2::ToVec4(const float z, const float w) const { return Vec4(x, y, z, w); }

	Vec2 Vec3::ToVec2() const { return Vec2(x, y); }
	Vec4 Vec3::ToVec4(const float w) const { return Vec4(x, y, z, w); }

	Vec2 Vec3::XY() const { return Vec2(x, y); }
	Vec2 Vec3::XZ() const { return Vec2(x, z); }
	Vec2 Vec3::YZ() const { return Vec2(y, z); }

	Vec2 Vec4::ToVec2() const { return Vec2(x, y); }
	Vec3 Vec4::ToVec3() const { return Vec3(x, y, z); }

	Vec2 Vec4::XY() const { return Vec2(x, y); }
	Vec2 Vec4::ZW() const { return Vec2(z, w); }
	Vec3 Vec4::XYZ() const { return Vec3(x, y, z); }
	Vec3 Vec4::XYW() const { return Vec3(x, y, w); }
	Vec3 Vec4::XZW() const { return Vec3(x, z, w); }
	Vec3 Vec4::YZW() const { return Vec3(y, z, w); }

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

	IntVec3 IntVec2::ToIntVec3(const int z) const { return IntVec3(x, y, z); }
	IntVec4 IntVec2::ToIntVec4(const int z, const int w) const { return IntVec4(x, y, z, w); }

	IntVec2 IntVec3::ToIntVec2() const { return IntVec2(x, y); }
	IntVec4 IntVec3::ToIntVec4(const int w) const { return IntVec4(x, y, z, w); }

	IntVec2 IntVec3::XY() const { return IntVec2(x, y); }
	IntVec2 IntVec3::XZ() const { return IntVec2(x, z); }
	IntVec2 IntVec3::YZ() const { return IntVec2(y, z); }

	IntVec2 IntVec4::ToIntVec2() const { return IntVec2(x, y); }
	IntVec3 IntVec4::ToIntVec3() const { return IntVec3(x, y, z); }

	IntVec2 IntVec4::XY() const { return IntVec2(x, y); }
	IntVec2 IntVec4::ZW() const { return IntVec2(z, w); }
	IntVec3 IntVec4::XYZ() const { return IntVec3(x, y, z); }
	IntVec3 IntVec4::XYW() const { return IntVec3(x, y, w); }
	IntVec3 IntVec4::XZW() const { return IntVec3(x, z, w); }
	IntVec3 IntVec4::YZW() const { return IntVec3(y, z, w); }

}