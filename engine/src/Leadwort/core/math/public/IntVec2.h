#pragma once

#include <glm/glm.hpp>
#include <iosfwd>
#include <sstream>
#include <string>

namespace Leadwort {

struct IntVec2 {
	int x { 0 };
	int y { 0 };

	explicit constexpr IntVec2() = default;
	explicit constexpr IntVec2(const int x, const int y) : x(x), y(y) {}
	explicit constexpr IntVec2(const int scalar) : x(scalar), y(scalar) {}

	// glm interop
	explicit constexpr IntVec2(const glm::ivec2& v) : x(v.x), y(v.y) {}
	explicit constexpr operator glm::ivec2() const { return { x, y }; }

	// Arithmetic
	constexpr IntVec2 operator+(const IntVec2& o) const { return IntVec2(x + o.x, y + o.y); }
	constexpr IntVec2 operator-(const IntVec2& o) const { return IntVec2(x - o.x, y - o.y); }
	constexpr IntVec2 operator*(const int s) const { return IntVec2(x * s,   y * s  ); }
	constexpr IntVec2 operator/(const int s) const { return IntVec2(x / s,   y / s  ); }
	constexpr IntVec2 operator-()              const { return IntVec2(-x, -y); }

	constexpr IntVec2& operator+=(const IntVec2& o) { x += o.x; y += o.y; return *this; }
	constexpr IntVec2& operator-=(const IntVec2& o) { x -= o.x; y -= o.y; return *this; }
	constexpr IntVec2& operator*=(const int s) { x *= s;   y *= s;   return *this; }
	constexpr IntVec2& operator/=(const int s) { x /= s;   y /= s;   return *this; }

	constexpr bool operator==(const IntVec2& o) const { return x == o.x && y == o.y; }
	constexpr bool operator!=(const IntVec2& o) const { return !(*this == o); }

	// Math
	[[nodiscard]] float Length()            const { return glm::length(glm::vec2(x, y)); }
	[[nodiscard]] int   LengthSq()          const { return x*x + y*y; }
	[[nodiscard]] int   Dot(const IntVec2& o)  const { return x*o.x + y*o.y; }

	// Swizzles
	[[nodiscard]] constexpr IntVec2 YX() const { return IntVec2(y, x); }
	[[nodiscard]] constexpr IntVec2 XX() const { return IntVec2(x, x); }
	[[nodiscard]] constexpr IntVec2 YY() const { return IntVec2(y, y); }

	// Conversions
	[[nodiscard]] struct IntVec3 ToIntVec3(int z = 0) const;
	[[nodiscard]] struct IntVec4 ToIntVec4(int z = 0, int w = 0) const;

	// Constants
	static constexpr IntVec2 Zero()  { return IntVec2(0, 0); }
	static constexpr IntVec2 One()   { return IntVec2(1, 1); }
	static constexpr IntVec2 Up()    { return IntVec2(0, 1); }
	static constexpr IntVec2 Down()  { return IntVec2(0,-1); }
	static constexpr IntVec2 Right() { return IntVec2(1, 0); }
	static constexpr IntVec2 Left()  { return IntVec2(-1, 0); }

	[[nodiscard]] std::string ToString() const {
		std::ostringstream oss;
		oss << "IntVec2(" << x << ", " << y << ")";
		return oss.str();
	}
};

inline IntVec2 operator*(const int s, const IntVec2& v) { return v * s; }

}