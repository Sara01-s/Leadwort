#pragma once

#include "Vec2.h"
#include "Vec3.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Engine {

struct Quat {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

	constexpr Quat() = default;
	constexpr Quat(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}

	// glm interop
	explicit Quat(const glm::quat& q) : x(q.x), y(q.y), z(q.z), w(q.w) {}
	explicit operator glm::quat() const { return { w, x, y, z }; }

	// From euler angles in degrees (XYZ order)
	static Quat FromEuler(const float pitchDeg, const float yawDeg, const float rollDeg) {
		constexpr float deg2rad = glm::pi<float>() / 180.0f;
		return Quat(glm::quat(glm::vec3(pitchDeg * deg2rad, yawDeg * deg2rad, rollDeg * deg2rad)));
	}

	static Quat FromEuler(const Vec3& degrees) {
		return FromEuler(degrees.x, degrees.y, degrees.z);
	}

	// To euler angles in degrees
	[[nodiscard]] Vec3 ToEuler() const {
		constexpr float rad2deg = 180.0f / glm::pi<float>();
		const glm::vec3 e = glm::eulerAngles(glm::quat(*this));
		return Vec3(e.x * rad2deg, e.y * rad2deg, e.z * rad2deg);
	}

	// From axis-angle (angle in degrees)
	static Quat FromAngleAxis(const float degrees, const Vec3& axis) {
		constexpr float deg2rad = glm::pi<float>() / 180.0f;
		return Quat(glm::angleAxis(degrees * deg2rad, glm::vec3(axis)));
	}

	// Arithmetic
	Quat operator*(const Quat& o) const { return Quat(glm::quat(*this) * glm::quat(o)); }
	Quat& operator*=(const Quat& o)    { *this = *this * o; return *this; }

	bool operator==(const Quat& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
	bool operator!=(const Quat& o) const { return !(*this == o); }

	// Math
	[[nodiscard]] Quat  Normalized()         const { return Quat(glm::normalize(glm::quat(*this))); }
	[[nodiscard]] Quat  Conjugate()          const { return Quat(glm::conjugate(glm::quat(*this))); }
	[[nodiscard]] Quat  Inverse()            const { return Quat(glm::inverse(glm::quat(*this))); }
	[[nodiscard]] float Dot(const Quat& o)   const { return glm::dot(glm::quat(*this), glm::quat(o)); }
	[[nodiscard]] Quat  Slerp(const Quat& o, const float t) const { return Quat(glm::slerp(glm::quat(*this), glm::quat(o), t)); }
	[[nodiscard]] Vec3 operator*(const Vec3& v) const { return Rotate(v); }
	[[nodiscard]] Vec2 operator*(const Vec2& v) const { const Vec3 v3(v.x, v.y, 0.0f); return Vec2(Rotate(v3).x, Rotate(v3).y); }


	[[nodiscard]] static Quat LookRotation(const Vec3& forward, const Vec3& up = Vec3::Up());
	[[nodiscard]] static Quat FromMatrix(const Mat4& matrix);

	// Rotate a vector
	[[nodiscard]] Vec3 Rotate(const Vec3& v) const { return Vec3(glm::rotate(glm::quat(*this), glm::vec3(v))); }

	// Direction vectors
	[[nodiscard]] Vec3 Forward() const { return Rotate(Vec3::Forward()); }
	[[nodiscard]] Vec3 Right()   const { return Rotate(Vec3::Right()); }
	[[nodiscard]] Vec3 Up()      const { return Rotate(Vec3::Up()); }

	// Constants
	static constexpr Quat Identity() { return { 0, 0, 0, 1 }; }

	[[nodiscard]] std::string ToString() const {
		std::ostringstream oss;
		oss << "Quat(" << w << ", " << y << ", " << z << ", " << x << ")";
		return oss.str();
	}
};

}