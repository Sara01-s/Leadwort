#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <sstream>
#include <string>

namespace Leadwort {

struct IntVec3 {
	int x { 0 };
	int y { 0 };
	int z { 0 };

	explicit constexpr IntVec3() = default;
	explicit constexpr IntVec3(const int x, const int y, const int z) : x(x), y(y), z(z) {}
	explicit constexpr IntVec3(const int scalar) : x(scalar), y(scalar), z(scalar) {}

	// glm interop
	explicit constexpr IntVec3(const glm::ivec3& v) : x(v.x), y(v.y), z(v.z) {}
	explicit constexpr operator glm::ivec3() const { return { x, y, z }; }

	// Arithmetic
	constexpr IntVec3 operator+(const IntVec3& o) const { return IntVec3(x + o.x, y + o.y, z + o.z); }
	constexpr IntVec3 operator-(const IntVec3& o) const { return IntVec3(x - o.x, y - o.y, z - o.z); }
	constexpr IntVec3 operator*(const int s) const { return IntVec3(x * s,   y * s,   z * s  ); }
	constexpr IntVec3 operator*(const IntVec3& o) const { return IntVec3(x * o.x, y * o.y, z * o.z); }
	constexpr IntVec3 operator/(const int s) const { return IntVec3(x / s,   y / s,   z / s  ); }
	constexpr IntVec3 operator/(const IntVec3& o) const { return IntVec3(x / o.x, y / o.y, z / o.z); }
	constexpr IntVec3 operator-()              const { return IntVec3(-x, -y, -z); }

	constexpr IntVec3& operator+=(const IntVec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
	constexpr IntVec3& operator-=(const IntVec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
	constexpr IntVec3& operator*=(const int s) { x *= s;   y *= s;   z *= s;   return *this; }
	constexpr IntVec3& operator*=(const IntVec3& o) { x *= o.x; y *= o.y; z *= o.z; return *this; }
	constexpr IntVec3& operator/=(const int s) { x /= s;   y /= s;   z /= s;   return *this; }
	constexpr IntVec3& operator/=(const IntVec3& o) { x /= o.x; y /= o.y; z /= o.z; return *this; }

	constexpr bool operator==(const IntVec3& o) const { return x == o.x && y == o.y && z == o.z; }
	constexpr bool operator!=(const IntVec3& o) const { return !(*this == o); }

	// Math
	[[nodiscard]] float   Length()              const { return glm::length(glm::vec3(x, y, z)); }
	[[nodiscard]] int     LengthSquared()       const { return x*x + y*y + z*z; }
	[[nodiscard]] int     Dot(const IntVec3& o) const { return x*o.x + y*o.y + z*o.z; }
	[[nodiscard]] IntVec3 Cross(const IntVec3& o) const {
		return IntVec3(
			y * o.z - z * o.y,
			z * o.x - x * o.z,
			x * o.y - y * o.x
		);
	}

	// Conversions
	[[nodiscard]] struct IntVec2 ToIntVec2() const;
	[[nodiscard]] struct IntVec4 ToIntVec4(int w = 0) const;
	[[nodiscard]] int* ToPtr() { return &x; }

	// Swizzles — same type
	[[nodiscard]] constexpr IntVec3 XZY() const { return IntVec3(x, z, y); }
	[[nodiscard]] constexpr IntVec3 YXZ() const { return IntVec3(y, x, z); }
	[[nodiscard]] constexpr IntVec3 ZXY() const { return IntVec3(z, x, y); }
	[[nodiscard]] constexpr IntVec3 ZYX() const { return IntVec3(z, y, x); }
	[[nodiscard]] constexpr IntVec3 XXX() const { return IntVec3(x, x, x); }
	[[nodiscard]] constexpr IntVec3 YYY() const { return IntVec3(y, y, y); }
	[[nodiscard]] constexpr IntVec3 ZZZ() const { return IntVec3(z, z, z); }

	// Swizzles — cross type (defined in VecConversions.cpp)
	[[nodiscard]] struct IntVec2 XY() const;
	[[nodiscard]] struct IntVec2 XZ() const;
	[[nodiscard]] struct IntVec2 YZ() const;

	// Constants
	static constexpr IntVec3 Zero()    { return IntVec3( 0,  0,  0); }
	static constexpr IntVec3 One()     { return IntVec3( 1,  1,  1); }
	static constexpr IntVec3 Up()      { return IntVec3( 0,  1,  0); }
	static constexpr IntVec3 Down()    { return IntVec3( 0, -1,  0); }
	static constexpr IntVec3 Right()   { return IntVec3( 1,  0,  0); }
	static constexpr IntVec3 Left()    { return IntVec3(-1,  0,  0); }
	static constexpr IntVec3 Forward() { return IntVec3( 0,  0,  1); }
	static constexpr IntVec3 Back()    { return IntVec3( 0,  0, -1); }

	[[nodiscard]] std::string ToString() const {
		std::ostringstream oss;
		oss << "IntVec3(" << x << ", " << y << ", " << z << ")";
		return oss.str();
	}
};

inline IntVec3 operator*(const int s, const IntVec3& v) { return v * s; }

}