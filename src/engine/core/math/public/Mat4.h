#pragma once

#include "Mat3.h"
#include "Quat.h"
#include "Vec3.h"
#include "Vec4.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <sstream>
#include <string>

namespace Engine {

struct Mat4 {

    constexpr Mat4() : m_Data(1.0f) {}
    constexpr explicit Mat4(const glm::mat4& m) : m_Data(m) {}
	constexpr Mat4(
		const float m00, const float m01, const float m02, const float m03,
		const float m10, const float m11, const float m12, const float m13,
		const float m20, const float m21, const float m22, const float m23,
		const float m30, const float m31, const float m32, const float m33
	)
		: m_Data(
			m00, m10, m20, m30,
			m01, m11, m21, m31,
			m02, m12, m22, m32,
			m03, m13, m23, m33
		)
	{}

	explicit operator glm::mat4() const { return m_Data; }

    // Arithmetic
    Mat4  operator*(const Mat4& o) const { return Mat4(m_Data * o.m_Data); }
    Vec4  operator*(const Vec4& v) const { return Vec4(m_Data * glm::vec4(v)); }
    Mat4& operator*=(const Mat4& o)      { m_Data *= o.m_Data; return *this; }
	glm::vec4& operator[](const int col) { return m_Data[col]; }
	const glm::vec4& operator[](const int col) const { return m_Data[col]; }

    bool operator==(const Mat4& o) const { return m_Data == o.m_Data; }
    bool operator!=(const Mat4& o) const { return !(*this == o); }

    // Math
    [[nodiscard]] Mat4  Transposed()  const { return Mat4(glm::transpose(m_Data)); }
    [[nodiscard]] Mat4  Inversed()    const { return Mat4(glm::inverse(m_Data)); }
    [[nodiscard]] float Determinant() const { return glm::determinant(m_Data); }
    [[nodiscard]] Mat3  ToMat3()      const { return Mat3(glm::mat3(m_Data)); }

    // Column/row access
    [[nodiscard]] Vec4 GetColumn(const int i) const { return Vec4(m_Data[i]); }
	void SetColumn(const int i, const Vec4& column) { m_Data[i] = glm::vec4(column); }

    // Decompose
	[[nodiscard]] Vec3 GetTranslation() const {
    	return Vec3(m_Data[3]);
    }

    [[nodiscard]] Quat GetRotation() const {
        glm::vec3 pos, scale, skew;
        glm::vec4 perspective;
        glm::quat rot;
        glm::decompose(m_Data, scale, rot, pos, skew, perspective);

        return Quat(rot);
    }

    [[nodiscard]] Vec3 GetScale() const {
        return Vec3(
            glm::length(glm::vec3(m_Data[0])),
            glm::length(glm::vec3(m_Data[1])),
            glm::length(glm::vec3(m_Data[2]))
        );
    }

    // Transform a point (w=1) vs direction (w=0)
    [[nodiscard]] Vec3 TransformPoint(const Vec3& p)     const { return Vec3(glm::vec3(m_Data * glm::vec4(glm::vec3(p), 1.0f))); }
    [[nodiscard]] Vec3 TransformDirection(const Vec3& d) const { return Vec3(glm::vec3(m_Data * glm::vec4(glm::vec3(d), 0.0f))); }

    // OpenGL upload
    [[nodiscard]] const float* ToPtr() const { return glm::value_ptr(m_Data); }

    // Factories
    static Mat4 Identity() { return Mat4(glm::mat4(1.0f)); }
    static Mat4 Zero()     { return Mat4(glm::mat4(0.0f)); }

    [[nodiscard]] static Mat4 Translate(const Vec3& t) {
        return Mat4(glm::translate(glm::mat4(1.0f), glm::vec3(t)));
    }

    [[nodiscard]] static Mat4 Rotate(const Quat& q) {
        return Mat4(glm::mat4_cast(glm::quat(q)));
    }

    [[nodiscard]] static Mat4 Scale(const Vec3& s) {
        return Mat4(glm::scale(glm::mat4(1.0f), glm::vec3(s)));
    }

    [[nodiscard]] static Mat4 TRS(const Vec3& t, const Quat& r, const Vec3& s) {
        return Translate(t) * Rotate(r) * Scale(s);
    }

    [[nodiscard]] static Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        return Mat4(glm::lookAtLH(glm::vec3(eye), glm::vec3(center), glm::vec3(up)));
    }

    [[nodiscard]] static Mat4 PerspectiveRH(const float fovDeg, const float zNear, const float zFar, const float aspect) {
        return Mat4(glm::perspective(glm::radians(fovDeg), aspect, zNear, zFar));
    }

	[[nodiscard]] static Mat4 PerspectiveLH(const float fovDeg, const float zNear, const float zFar, const float aspect) {
    	return Mat4(glm::perspectiveLH(glm::radians(fovDeg), aspect, zNear, zFar));
    }

    [[nodiscard]] static Mat4 Orthographic(
    	const float left, const float right,
        const float bottom, const float top,
        const float zNear,  const float zFar
    ) {
        return Mat4(glm::orthoLH(left, right, bottom, top, zNear, zFar));
    }

	[[nodiscard]] static Mat4 FromAxes(const Vec3& right, const Vec3& up, const Vec3& forward) {
    	Mat4 result(1.0f);

    	result.m_Data[0] = glm::vec4(glm::vec3(right),   0.0f);
    	result.m_Data[1] = glm::vec4(glm::vec3(up),      0.0f);
    	result.m_Data[2] = glm::vec4(glm::vec3(forward), 0.0f);
    	result.m_Data[3] = glm::vec4(0.0f, 0.0f, 0.0f,   1.0f);

    	return result;
    }

	[[nodiscard]] const glm::mat4& GetInternalMatrix() const { return m_Data; }

	[[nodiscard]] std::string ToString() const {
    	std::ostringstream oss;
    	oss << "Mat4(\n";
    	for (int row = 0; row < 4; ++row) {
    		oss << "  [ ";
    		for (int col = 0; col < 4; ++col) {
    			oss << (*this)[col][row] << (col < 3 ? ", " : " ");
    		}
    		oss << "]\n";
    	}
    	oss << ")";
    	return oss.str();
    }

private:
    glm::mat4 m_Data;
};

inline Mat3 Mat3::NormalMatrix(const Mat4& model) {
    return Mat3(glm::mat3(glm::transpose(glm::inverse(glm::mat4(model)))));
}

}
