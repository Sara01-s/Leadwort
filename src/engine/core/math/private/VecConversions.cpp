#define GLM_ENABLE_EXPERIMENTAL
#include "../public/Vec2.h"
#include "../public/Vec3.h"
#include "../public/Vec4.h"
#include "engine/core/math/public/Mat4.h"

namespace Engine {

// ── Vec2 conversions ──────────────────────────────────────────────────────
Vec3 Vec2::ToVec3(const float z) const { return Vec3(x, y, z); }
Vec4 Vec2::ToVec4(const float z, const float w) const { return Vec4(x, y, z, w); }

// ── Vec3 conversions ──────────────────────────────────────────────────────
Vec2 Vec3::ToVec2() const { return Vec2(x, y); }
Vec4 Vec3::ToVec4(const float w) const { return Vec4(x, y, z, w); }

// ── Vec3 swizzles ─────────────────────────────────────────────────────────
Vec2 Vec3::XY() const { return Vec2(x, y); }
Vec2 Vec3::XZ() const { return Vec2(x, z); }
Vec2 Vec3::YZ() const { return Vec2(y, z); }

// ── Vec4 conversions ──────────────────────────────────────────────────────
Vec2 Vec4::ToVec2() const { return Vec2(x, y); }
Vec3 Vec4::ToVec3() const { return Vec3(x, y, z); }

// ── Vec4 swizzles ─────────────────────────────────────────────────────────
Vec2 Vec4::XY() const { return Vec2(x, y); }
Vec2 Vec4::ZW() const { return Vec2(z, w); }
Vec3 Vec4::XYZ() const { return Vec3(x, y, z); }
Vec3 Vec4::XYW() const { return Vec3(x, y, w); }
Vec3 Vec4::XZW() const { return Vec3(x, z, w); }
Vec3 Vec4::YZW() const { return Vec3(y, z, w); }

// ── Quat conversions ──────────────────────────────────────────────────────
Mat4 Quat::ToMat4() const { return Mat4(glm::mat4_cast(glm::quat(*this))); }

}