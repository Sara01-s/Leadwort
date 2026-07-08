#pragma once

#include <glm/glm.hpp>

namespace Engine {

struct Vec4 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

	explicit constexpr Vec4() = default;
	explicit constexpr Vec4(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}
	explicit constexpr Vec4(const float scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}

	// glm interop
	explicit constexpr Vec4(const glm::vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
	explicit constexpr operator glm::vec4() const { return glm::vec4(x, y, z, w); }

	// Arithmetic
	constexpr Vec4 operator+(const Vec4& o) const { return Vec4(x + o.x, y + o.y, z + o.z, w + o.w); }
	constexpr Vec4 operator-(const Vec4& o) const { return Vec4(x - o.x, y - o.y, z - o.z, w - o.w); }
	constexpr Vec4 operator*(const float s) const { return Vec4(x * s,   y * s,   z * s,   w * s  ); }
	constexpr Vec4 operator/(const float s) const { return Vec4(x / s,   y / s,   z / s,   w / s  ); }
	constexpr Vec4 operator-()              const { return Vec4(-x, -y, -z, -w ); }

	constexpr Vec4& operator+=(const Vec4& o) { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }
	constexpr Vec4& operator-=(const Vec4& o) { x -= o.x; y -= o.y; z -= o.z; w -= o.w; return *this; }
	constexpr Vec4& operator*=(const float s) { x *= s;   y *= s;   z *= s;   w *= s;   return *this; }
	constexpr Vec4& operator/=(const float s) { x /= s;   y /= s;   z /= s;   w /= s;   return *this; }

	constexpr bool operator==(const Vec4& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
	constexpr bool operator!=(const Vec4& o) const { return !(*this == o); }

	// Math
	[[nodiscard]] float Length()            const { return glm::length(glm::vec4(*this)); }
	[[nodiscard]] float LengthSq()          const { return x*x + y*y + z*z + w*w; }
	[[nodiscard]] Vec4  Normalized()        const { return Vec4(glm::normalize(glm::vec4(*this))); }
	[[nodiscard]] float Dot(const Vec4& o)  const { return glm::dot(glm::vec4(*this), glm::vec4(o)); }
	[[nodiscard]] Vec4  Lerp(const Vec4& o, const float t) const { return Vec4(glm::mix(glm::vec4(*this), glm::vec4(o), t)); }

	// Swizzles — same type
	[[nodiscard]] constexpr Vec4 XYZW() const { return Vec4(x, y, z, w); }
	[[nodiscard]] constexpr Vec4 WZYX() const { return Vec4(w, z, y, x); }
	[[nodiscard]] constexpr Vec4 XXXX() const { return Vec4(x, x, x, x); }
	[[nodiscard]] constexpr Vec4 YYYY() const { return Vec4(y, y, y, y); }
	[[nodiscard]] constexpr Vec4 ZZZZ() const { return Vec4(z, z, z, z); }
	[[nodiscard]] constexpr Vec4 WWWW() const { return Vec4(w, w, w, w); }

	// Swizzles — cross type (defined in VecConversions.cpp)
	[[nodiscard]] Vec2 XY()  const;
	[[nodiscard]] Vec2 ZW()  const;
	[[nodiscard]] Vec3 XYZ() const;
	[[nodiscard]] Vec3 XYW() const;
	[[nodiscard]] Vec3 XZW() const;
	[[nodiscard]] Vec3 YZW() const;

	// Conversions
	[[nodiscard]] Vec2 ToVec2() const;
	[[nodiscard]] Vec3 ToVec3() const;
	[[nodiscard]] const float* ToPtr() const { return &x; }

	// Constants
	static constexpr Vec4 Zero()     { return Vec4(0, 0, 0, 0); }
	static constexpr Vec4 One()      { return Vec4(1, 1, 1, 1); }
	static constexpr Vec4 Identity() { return Vec4(0, 0, 0, 1); }

	[[nodiscard]] std::string ToString() const {
		std::ostringstream oss;
		oss << "Vec4(" << x << ", " << y << ", " << z << ", " << w << ")";
		return oss.str();
	}
};

inline Vec4 operator*(const float s, const Vec4& v) { return v * s; }

}