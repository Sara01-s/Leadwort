#pragma once

#include "Vec3.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_access.hpp>

namespace Engine {

struct Mat4;

struct Mat3 {
	Mat3() : m_Data(1.0f) {}
	explicit Mat3(const glm::mat3& m) : m_Data(m) {}
	explicit operator glm::mat3() const { return m_Data; }

	// Arithmetic
	Mat3 operator*(const Mat3& o) const { return Mat3(m_Data * o.m_Data); }
	Vec3 operator*(const Vec3& v) const { return Vec3(m_Data * glm::vec3(v)); }
	Mat3& operator*=(const Mat3& o)     { m_Data *= o.m_Data; return *this; }

	bool operator==(const Mat3& o) const { return m_Data == o.m_Data; }
	bool operator!=(const Mat3& o) const { return !(*this == o); }

	// Math
	[[nodiscard]] Mat3  Transposed()  const { return Mat3(glm::transpose(m_Data)); }
	[[nodiscard]] Mat3  Inversed()    const { return Mat3(glm::inverse(m_Data)); }
	[[nodiscard]] float Determinant() const { return glm::determinant(m_Data); }

	// Normal matrix from a model Mat4 (defined in Mat4.h after Mat4 is complete)
	[[nodiscard]] static Mat3 NormalMatrix(const Mat4& model);

	// Column/row access
	[[nodiscard]] Vec3 GetColumn(const int i) const { return Vec3(m_Data[i]); }
	[[nodiscard]] Vec3 GetRow(const int i)    const { return Vec3(glm::row(m_Data, i)); }

	// OpenGL upload
	[[nodiscard]] const float* ToPtr() const { return glm::value_ptr(m_Data); }

	// Constants
	static Mat3 Identity() { return Mat3(glm::mat3(1.0f)); }
	static Mat3 Zero()     { return Mat3(glm::mat3(0.0f)); }

private:
	glm::mat3 m_Data;
};

}