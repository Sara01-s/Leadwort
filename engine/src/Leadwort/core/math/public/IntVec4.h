#pragma once

#include <glm/glm.hpp>

namespace Leadwort {

struct IntVec2;
struct IntVec3;

struct IntVec4 {
	int x { 0 };
	int y { 0 };
	int z { 0 };
	int w { 0 };

	explicit constexpr IntVec4() = default;
	explicit constexpr IntVec4(const int x, const int y, const int z, const int w) : x(x), y(y), z(z), w(w) {}
	explicit constexpr IntVec4(const int scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}

	// glm interop
	explicit constexpr IntVec4(const glm::ivec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
	explicit constexpr operator glm::ivec4() const { return glm::ivec4(x, y, z, w); }

	// Arithmetic
	constexpr IntVec4 operator+(const IntVec4& o) const { return IntVec4(x + o.x, y + o.y, z + o.z, w + o.w); }
	constexpr IntVec4 operator-(const IntVec4& o) const { return IntVec4(x - o.x, y - o.y, z - o.z, w - o.w); }
	constexpr IntVec4 operator*(const int s) const { return IntVec4(x * s,   y * s,   z * s,   w * s  ); }
	constexpr IntVec4 operator/(const int s) const { return IntVec4(x / s,   y / s,   z / s,   w / s  ); }
	constexpr IntVec4 operator-()              const { return IntVec4(-x, -y, -z, -w ); }

	constexpr IntVec4& operator+=(const IntVec4& o) { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }
	constexpr IntVec4& operator-=(const IntVec4& o) { x -= o.x; y -= o.y; z -= o.z; w -= o.w; return *this; }
	constexpr IntVec4& operator*=(const int s) { x *= s;   y *= s;   z *= s;   w *= s;   return *this; }
	constexpr IntVec4& operator/=(const int s) { x /= s;   y /= s;   z /= s;   w /= s;   return *this; }

	constexpr bool operator==(const IntVec4& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
	constexpr bool operator!=(const IntVec4& o) const { return !(*this == o); }

	// Math
	[[nodiscard]] float Length()            const { return glm::length(glm::vec4(x, y, z, w)); }
	[[nodiscard]] int   LengthSq()          const { return x*x + y*y + z*z + w*w; }
	[[nodiscard]] int   Dot(const IntVec4& o)  const { return x*o.x + y*o.y + z*o.z + w*o.w; }

	// Swizzles — same type
	[[nodiscard]] constexpr IntVec4 XYZW() const { return IntVec4(x, y, z, w); }
	[[nodiscard]] constexpr IntVec4 WZYX() const { return IntVec4(w, z, y, x); }
	[[nodiscard]] constexpr IntVec4 XXXX() const { return IntVec4(x, x, x, x); }
	[[nodiscard]] constexpr IntVec4 YYYY() const { return IntVec4(y, y, y, y); }
	[[nodiscard]] constexpr IntVec4 ZZZZ() const { return IntVec4(z, z, z, z); }
	[[nodiscard]] constexpr IntVec4 WWWW() const { return IntVec4(w, w, w, w); }

	// Swizzles — cross type (defined in VecConversions.cpp)
	[[nodiscard]] IntVec2 XY()  const;
	[[nodiscard]] IntVec2 ZW()  const;
	[[nodiscard]] IntVec3 XYZ() const;
	[[nodiscard]] IntVec3 XYW() const;
	[[nodiscard]] IntVec3 XZW() const;
	[[nodiscard]] IntVec3 YZW() const;

	// Conversions
	[[nodiscard]] IntVec2 ToIntVec2() const;
	[[nodiscard]] IntVec3 ToIntVec3() const;
	[[nodiscard]] const int* ToPtr() const { return &x; }

	// Constants
	static constexpr IntVec4 Zero()     { return IntVec4(0, 0, 0, 0); }
	static constexpr IntVec4 One()      { return IntVec4(1, 1, 1, 1); }
	static constexpr IntVec4 Identity() { return IntVec4(0, 0, 0, 1); }

	[[nodiscard]] std::string ToString() const {
		std::ostringstream oss;
		oss << "IntVec4(" << x << ", " << y << ", " << z << ", " << w << ")";
		return oss.str();
	}
};

inline IntVec4 operator*(const int s, const IntVec4& v) { return v * s; }

}