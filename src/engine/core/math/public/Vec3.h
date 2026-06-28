#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <sstream>
#include <string>

namespace Engine {

struct Vec3 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	explicit constexpr Vec3() = default;
	explicit constexpr Vec3(const float x, const float y, const float z) : x(x), y(y), z(z) {}
	explicit constexpr Vec3(const float scalar) : x(scalar), y(scalar), z(scalar) {}

	// glm interop
	explicit constexpr Vec3(const glm::vec3& v) : x(v.x), y(v.y), z(v.z) {}
	explicit constexpr operator glm::vec3() const { return { x, y, z }; }

	// Arithmetic
	constexpr Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
	constexpr Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
	constexpr Vec3 operator*(const float s) const { return Vec3(x * s,   y * s,   z * s  ); }
	constexpr Vec3 operator*(const Vec3& o) const { return Vec3(x * o.x, y * o.y, z * o.z); }
	constexpr Vec3 operator/(const float s) const { return Vec3(x / s,   y / s,   z / s  ); }
	constexpr Vec3 operator/(const Vec3& o) const { return Vec3(x / o.x, y / o.y, z / o.z); }
	constexpr Vec3 operator-()              const { return Vec3(-x, -y, -z); }

	constexpr Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
	constexpr Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
	constexpr Vec3& operator*=(const float s) { x *= s;   y *= s;   z *= s;   return *this; }
	constexpr Vec3& operator*=(const Vec3& o) { x *= o.x; y *= o.y; z *= o.z; return *this; }
	constexpr Vec3& operator/=(const float s) { x /= s;   y /= s;   z /= s;   return *this; }
	constexpr Vec3& operator/=(const Vec3& o) { x /= o.x; y /= o.y; z /= o.z; return *this; }

	constexpr bool operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
	constexpr bool operator!=(const Vec3& o) const { return !(*this == o); }

	// Math
	[[nodiscard]] float    Length()              const { return glm::length(glm::vec3(*this)); }
	[[nodiscard]] float    LengthSquared()       const { return x*x + y*y + z*z; }
	[[nodiscard]] Vec3     Normalized()          const { return Vec3(glm::normalize(glm::vec3(*this))); }
	[[nodiscard]] float    Dot(const Vec3& o)    const { return glm::dot(glm::vec3(*this), glm::vec3(o)); }
	[[nodiscard]] Vec3     Cross(const Vec3& o)  const { return Vec3(glm::cross(glm::vec3(*this), glm::vec3(o))); }
	[[nodiscard]] Vec3     Lerp(const Vec3& o, const float t) const { return Vec3(glm::mix(glm::vec3(*this), glm::vec3(o), t)); }

	// Conversions
	[[nodiscard]] struct Vec2 ToVec2() const;
	[[nodiscard]] struct Vec4 ToVec4(float w = 0.0f) const;
	[[nodiscard]] float* ToPtr() { return &x; }

	// Swizzles — same type
	[[nodiscard]] constexpr Vec3 XZY() const { return Vec3(x, z, y); }
	[[nodiscard]] constexpr Vec3 YXZ() const { return Vec3(y, x, z); }
	[[nodiscard]] constexpr Vec3 ZXY() const { return Vec3(z, x, y); }
	[[nodiscard]] constexpr Vec3 ZYX() const { return Vec3(z, y, x); }
	[[nodiscard]] constexpr Vec3 XXX() const { return Vec3(x, x, x); }
	[[nodiscard]] constexpr Vec3 YYY() const { return Vec3(y, y, y); }
	[[nodiscard]] constexpr Vec3 ZZZ() const { return Vec3(z, z, z); }

	// Swizzles — cross type (defined in VecConversions.cpp)
	[[nodiscard]] struct Vec2 XY() const;
	[[nodiscard]] struct Vec2 XZ() const;
	[[nodiscard]] struct Vec2 YZ() const;

	// Constants
	static constexpr Vec3 Zero()    { return Vec3( 0,  0,  0); }
	static constexpr Vec3 One()     { return Vec3( 1,  1,  1); }
	static constexpr Vec3 Up()      { return Vec3( 0,  1,  0); }
	static constexpr Vec3 Down()    { return Vec3( 0, -1,  0); }
	static constexpr Vec3 Right()   { return Vec3( 1,  0,  0); }
	static constexpr Vec3 Left()    { return Vec3(-1,  0,  0); }
	static constexpr Vec3 Forward() { return Vec3( 0,  0,  1); }
	static constexpr Vec3 Back()    { return Vec3( 0,  0, -1); }

	[[nodiscard]] std::string ToString() const {
		std::ostringstream oss;
		oss << "Vec3(" << x << ", " << y << ", " << z << ")";
		return oss.str();
	}
};

inline Vec3 operator*(const float s, const Vec3& v) { return v * s; }

}