#pragma once

#include <glm/glm.hpp>
#include <iosfwd>
#include <sstream>
#include <string>

namespace Engine {

struct Vec2 {
	float x = 0.0f;
	float y = 0.0f;

	explicit constexpr Vec2() = default;
	explicit constexpr Vec2(const float x, const float y) : x(x), y(y) {}
	explicit constexpr Vec2(const float scalar) : x(scalar), y(scalar) {}

	// glm interop
	explicit constexpr Vec2(const glm::vec2& v) : x(v.x), y(v.y) {}
	explicit constexpr operator glm::vec2() const { return { x, y }; }

	// Arithmetic
	constexpr Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
	constexpr Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
	constexpr Vec2 operator*(const float s) const { return Vec2(x * s,   y * s  ); }
	constexpr Vec2 operator/(const float s) const { return Vec2(x / s,   y / s  ); }
	constexpr Vec2 operator-()              const { return Vec2(-x, -y); }

	constexpr Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
	constexpr Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
	constexpr Vec2& operator*=(const float s) { x *= s;   y *= s;   return *this; }
	constexpr Vec2& operator/=(const float s) { x /= s;   y /= s;   return *this; }

	constexpr bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
	constexpr bool operator!=(const Vec2& o) const { return !(*this == o); }

	// Math
	[[nodiscard]] float Length()            const { return glm::length(glm::vec2(*this)); }
	[[nodiscard]] float LengthSq()          const { return x*x + y*y; }
	[[nodiscard]] Vec2  Normalized()        const { return Vec2(glm::normalize(glm::vec2(*this))); }
	[[nodiscard]] float Dot(const Vec2& o)  const { return glm::dot(glm::vec2(*this), glm::vec2(o)); }
	[[nodiscard]] Vec2  Lerp(const Vec2& o, const float t) const { return Vec2(glm::mix(glm::vec2(*this), glm::vec2(o), t)); }

	// Swizzles
	[[nodiscard]] constexpr Vec2 YX() const { return Vec2(y, x); }
	[[nodiscard]] constexpr Vec2 XX() const { return Vec2(x, x); }
	[[nodiscard]] constexpr Vec2 YY() const { return Vec2(y, y); }

	// Conversions
	[[nodiscard]] struct Vec3 ToVec3(float z = 0.0f) const;
	[[nodiscard]] struct Vec4 ToVec4(float z = 0.0f, float w = 0.0f) const;

	// Constants
	static constexpr Vec2 Zero()  { return Vec2(0, 0); }
	static constexpr Vec2 One()   { return Vec2(1, 1); }
	static constexpr Vec2 Up()    { return Vec2(0, 1); }
	static constexpr Vec2 Down()  { return Vec2(0,-1); }
	static constexpr Vec2 Right() { return Vec2(1, 0); }
	static constexpr Vec2 Left()  { return Vec2(1, 0); }

	[[nodiscard]] std::string ToString() const {
		std::ostringstream oss;
		oss << "Vec2(" << x << ", " << y << ")";
		return oss.str();
	}
};

inline Vec2 operator*(const float s, const Vec2& v) { return v * s; }

}