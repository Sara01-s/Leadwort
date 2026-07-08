#pragma once

#include "Mat3.h"
#include "Mat4.h"
#include "Quat.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <limits>
#include <numbers>

namespace Engine {

// ─────────────────────────────────────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────────────────────────────────────

inline constexpr float Pi        = std::numbers::pi_v<float>;
inline constexpr float Tau       = Pi * 2.0f;
inline constexpr float HalfPi    = Pi * 0.5f;
inline constexpr float InvPi     = 1.0f / Pi;
inline constexpr float Sqrt2     = std::numbers::sqrt2_v<float>;
inline constexpr float Sqrt3     = std::numbers::sqrt3_v<float>;
inline constexpr float Epsilon   = std::numeric_limits<float>::epsilon();
inline constexpr float Infinity  = std::numeric_limits<float>::infinity();
inline constexpr float NegInfinity = -std::numeric_limits<float>::infinity();
inline constexpr float Deg2Rad   = Pi / 180.0f;
inline constexpr float Rad2Deg   = 180.0f / Pi;
inline constexpr float GoldenRatio = std::numbers::phi_v<float>;
inline constexpr int IntInfinity = std::numeric_limits<int>::infinity();

// ─────────────────────────────────────────────────────────────────────────────
//  Scalar
// ─────────────────────────────────────────────────────────────────────────────

[[nodiscard]] inline float ToRadians(const float degrees) { return degrees * Deg2Rad; }
[[nodiscard]] inline float ToDegrees(const float radians) { return radians * Rad2Deg; }

[[nodiscard]] inline float Clamp(const float v, const float lo, const float hi)  { return std::clamp(v, lo, hi); }
[[nodiscard]] inline float Clamp01(const float v)                                { return Clamp(v, 0.0f, 1.0f); }
[[nodiscard]] inline float Saturate(const float v)                               { return Clamp01(v); }

[[nodiscard]] inline float Lerp(const float a, const float b, const float t)     { return a + (b - a) * t; }
[[nodiscard]] inline float InverseLerp(const float a, const float b, const float v) { return (v - a) / (b - a); }
[[nodiscard]] inline float Remap(const float v,
    const float inMin,  const float inMax,
    const float outMin, const float outMax) {
    return Lerp(outMin, outMax, InverseLerp(inMin, inMax, v));
}

[[nodiscard]] inline float SmoothStep(const float edge0, const float edge1, const float x) {
    const float t = Clamp01((x - edge0) / (edge1 - edge0));
    return t * t * (3.0f - 2.0f * t);
}

[[nodiscard]] inline float SmootherStep(const float edge0, const float edge1, const float x) {
    const float t = Clamp01((x - edge0) / (edge1 - edge0));
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

[[nodiscard]] inline float Sign(const float v)               { return v < 0.0f ? -1.0f : (v > 0.0f ? 1.0f : 0.0f); }
[[nodiscard]] inline float Abs(const float v)                { return std::abs(v); }
[[nodiscard]] inline float Floor(const float v)              { return std::floor(v); }
[[nodiscard]] inline float Ceil(const float v)               { return std::ceil(v); }
[[nodiscard]] inline float Round(const float v)              { return std::round(v); }
[[nodiscard]] inline float Frac(const float v)               { return v - std::floor(v); }
[[nodiscard]] inline float Mod(const float v, const float m) { return std::fmod(v, m); }
[[nodiscard]] inline float Sqrt(const float v)               { return std::sqrt(v); }
[[nodiscard]] inline float Pow(const float b, const float e) { return std::pow(b, e); }
[[nodiscard]] inline float Log(const float v)                { return std::log(v); }
[[nodiscard]] inline float Log2(const float v)               { return std::log2(v); }
[[nodiscard]] inline float Exp(const float v)                { return std::exp(v); }
[[nodiscard]] inline float Min(const float a, const float b) { return std::min(a, b); }
[[nodiscard]] inline float Max(const float a, const float b) { return std::max(a, b); }

[[nodiscard]] inline float Sin(const float rad)              { return std::sin(rad); }
[[nodiscard]] inline float Cos(const float rad)              { return std::cos(rad); }
[[nodiscard]] inline float Tan(const float rad)              { return std::tan(rad); }
[[nodiscard]] inline float Asin(const float v)               { return std::asin(v); }
[[nodiscard]] inline float Acos(const float v)               { return std::acos(v); }
[[nodiscard]] inline float Atan(const float v)               { return std::atan(v); }
[[nodiscard]] inline float Atan2(const float y, const float x) { return std::atan2(y, x); }

[[nodiscard]] inline float MoveTowards(const float current, const float target, const float maxDelta) {
    const float diff = target - current;
    if (Abs(diff) <= maxDelta) return target;
    return current + Sign(diff) * maxDelta;
}

[[nodiscard]] inline float PingPong(const float t, const float length) {
    const float mod = Mod(t, 2.0f * length);
    return length - Abs(mod - length);
}

[[nodiscard]] inline float Repeat(const float t, const float length) {
    return Clamp(t - Floor(t / length) * length, 0.0f, length);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Algorithms & Predicates
// ─────────────────────────────────────────────────────────────────────────────

// Checks if at least one of the provided conditions is true
// Usage example: if (Any(x > 1.0f, y > 1.0f, z > 1.0f)) ...
template<typename... Args>
[[nodiscard]] constexpr bool Any(Args... args) { return (... || args); }
// Checks if all provided conditions are true
template<typename... Args>
[[nodiscard]] constexpr bool All(Args... args) { return (... && args); }
// Returns the value with the largest absolute magnitude
[[nodiscard]] inline float MaxAbs(const float a, const float b) { return Abs(a) > Abs(b) ? a : b; }
// Returns the value with the smallest absolute magnitude
[[nodiscard]] inline float MinAbs(const float a, const float b) { return Abs(a) < Abs(b) ? a : b; }
// Maps a value from [min, max] range to a [0, 1] range
[[nodiscard]] inline float Normalize01(const float v, const float min, const float max) { return Clamp01((v - min) / (max - min)); }
// Calculates the average of two values
[[nodiscard]] inline float Average(const float a, const float b) { return (a + b) * 0.5f; }
// Checks if a value is within an inclusive range
[[nodiscard]] inline bool InRange(const float v, const float min, const float max) { return v >= min && v <= max; }
// Checks if a scalar is NaN (Not a Number)
[[nodiscard]] inline bool IsNaN(const float v) { return std::isnan(v); }
// Checks if any component of a Vec2 is NaN
[[nodiscard]] inline bool IsNaN(const Vec2& v) { return IsNaN(v.x) || IsNaN(v.y); }
// Checks if any component of a Vec3 is NaN
[[nodiscard]] inline bool IsNaN(const Vec3& v) { return IsNaN(v.x) || IsNaN(v.y) || IsNaN(v.z); }
// Checks if any component of a Vec4 is NaN
[[nodiscard]] inline bool IsNaN(const Vec4& v) { return IsNaN(v.x) || IsNaN(v.y) || IsNaN(v.z) || IsNaN(v.w); }
// Checks if any component of a Quat is NaN
[[nodiscard]] inline bool IsNaN(const Quat& q) { return IsNaN(q.x) || IsNaN(q.y) || IsNaN(q.z) || IsNaN(q.w); }
// Checks if a scalar is infinite
[[nodiscard]] inline bool IsInfinite(const float v) { return std::isinf(v); }
// Checks if a scalar is finite
[[nodiscard]] inline bool IsFinite(const float v) { return std::isfinite(v); }
[[nodiscard]] inline bool IsApproximately(const float a, const float b, const float eps = Epsilon) { return Abs(a - b) <= eps; }
// Wraps an angle to the [-Pi, Pi] range to prevent infinite accumulation
[[nodiscard]] inline float WrapAngle(float angle) {
	angle = Mod(angle + Pi, Tau);
	if (angle < 0.0f) {
		angle += Tau;
	}
	return angle - Pi;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Vec2
// ─────────────────────────────────────────────────────────────────────────────

[[nodiscard]] inline float Dot(const Vec2& a, const Vec2& b)           { return a.Dot(b); }
[[nodiscard]] inline float Length(const Vec2& v)                       { return v.Length(); }
[[nodiscard]] inline float LengthSquared(const Vec2& v)                     { return v.LengthSq(); }
[[nodiscard]] inline Vec2  Normalize(const Vec2& v)                    { return v.Normalized(); }
[[nodiscard]] inline Vec2  Lerp(const Vec2& a, const Vec2& b, float t) { return a.Lerp(b, t); }
[[nodiscard]] inline float Distance(const Vec2& a, const Vec2& b)      { return (b - a).Length(); }
[[nodiscard]] inline float DistanceSq(const Vec2& a, const Vec2& b)    { return (b - a).LengthSq(); }
[[nodiscard]] inline Vec2  Abs(const Vec2& v)                          { return Vec2(Abs(v.x), Abs(v.y)); }
[[nodiscard]] inline Vec2  Min(const Vec2& a, const Vec2& b)           { return Vec2(Min(a.x, b.x), Min(a.y, b.y)); }
[[nodiscard]] inline Vec2  Max(const Vec2& a, const Vec2& b)           { return Vec2(Max(a.x, b.x), Max(a.y, b.y)); }
[[nodiscard]] inline Vec2  Clamp(const Vec2& v, const Vec2& lo, const Vec2& hi) {
    return Vec2(Clamp(v.x, lo.x, hi.x), Clamp(v.y, lo.y, hi.y));
}

// Rotate a 2D vector by angle in radians
[[nodiscard]] inline Vec2 Rotate(const Vec2& v, const float rad) {
    const float c = Cos(rad), s = Sin(rad);
    return Vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}

// Angle between two 2D vectors in radians
[[nodiscard]] inline float Angle(const Vec2& a, const Vec2& b) {
    return Atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Vec3
// ─────────────────────────────────────────────────────────────────────────────

[[nodiscard]] inline float Dot(const Vec3& a, const Vec3& b)           { return a.Dot(b); }
[[nodiscard]] inline Vec3  Cross(const Vec3& a, const Vec3& b)         { return a.Cross(b); }
[[nodiscard]] inline float Length(const Vec3& v)                       { return v.Length(); }
[[nodiscard]] inline float LengthSquared(const Vec3& v)                     { return v.LengthSquared(); }
[[nodiscard]] inline Vec3  Normalize(const Vec3& v)                    { return v.Normalized(); }
[[nodiscard]] inline Vec3  Lerp(const Vec3& a, const Vec3& b, float t) { return a.Lerp(b, t); }
[[nodiscard]] inline float Distance(const Vec3& a, const Vec3& b)      { return (b - a).Length(); }
[[nodiscard]] inline float DistanceSq(const Vec3& a, const Vec3& b)    { return (b - a).LengthSquared(); }
[[nodiscard]] inline Vec3  Abs(const Vec3& v)                          { return Vec3(Abs(v.x), Abs(v.y), Abs(v.z)); }
[[nodiscard]] inline Vec3  Min(const Vec3& a, const Vec3& b)           { return Vec3(Min(a.x, b.x), Min(a.y, b.y), Min(a.z, b.z)); }
[[nodiscard]] inline Vec3  Max(const Vec3& a, const Vec3& b)           { return Vec3(Max(a.x, b.x), Max(a.y, b.y), Max(a.z, b.z)); }
[[nodiscard]] inline Vec3  Clamp(const Vec3& v, const Vec3& lo, const Vec3& hi) {
    return Vec3(Clamp(v.x, lo.x, hi.x), Clamp(v.y, lo.y, hi.y), Clamp(v.z, lo.z, hi.z));
}

[[nodiscard]] inline Vec3 Reflect(const Vec3& v, const Vec3& normal) {
    return Vec3(glm::reflect(glm::vec3(v), glm::vec3(normal)));
}

[[nodiscard]] inline Vec3 Refract(const Vec3& v, const Vec3& normal, const float eta) {
    return Vec3(glm::refract(glm::vec3(v), glm::vec3(normal), eta));
}

[[nodiscard]] inline Vec3 Project(const Vec3& v, const Vec3& onto) {
    return onto * (Dot(v, onto) / onto.LengthSquared());
}

[[nodiscard]] inline Vec3 Reject(const Vec3& v, const Vec3& onto) {
    return v - Project(v, onto);
}

// Angle between two Vec3 in radians
[[nodiscard]] inline float Angle(const Vec3& a, const Vec3& b) {
    return Acos(Clamp(Dot(Normalize(a), Normalize(b)), -1.0f, 1.0f));
}

// Signed angle around an axis in radians
[[nodiscard]] inline float SignedAngle(const Vec3& from, const Vec3& to, const Vec3& axis) {
    const float angle = Angle(from, to);
    return Sign(Dot(axis, Cross(from, to))) * angle;
}

[[nodiscard]] inline Vec3 MoveTowards(const Vec3& current, const Vec3& target, const float maxDelta) {
    const Vec3 diff = target - current;
    const float dist = diff.Length();
    if (dist <= maxDelta || dist < Epsilon) return target;
    return current + diff * (maxDelta / dist);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Vec4
// ─────────────────────────────────────────────────────────────────────────────

[[nodiscard]] inline float Dot(const Vec4& a, const Vec4& b)           { return a.Dot(b); }
[[nodiscard]] inline float Length(const Vec4& v)                       { return v.Length(); }
[[nodiscard]] inline Vec4  Normalize(const Vec4& v)                    { return v.Normalized(); }
[[nodiscard]] inline Vec4  Lerp(const Vec4& a, const Vec4& b, const float t) { return a.Lerp(b, t); }

// ─────────────────────────────────────────────────────────────────────────────
//  Quaternion
// ─────────────────────────────────────────────────────────────────────────────

[[nodiscard]] inline Quat  Normalize(const Quat& q)                      { return q.Normalized(); }
[[nodiscard]] inline Quat  Inverse(const Quat& q)                        { return q.Inverse(); }
[[nodiscard]] inline Quat  Slerp(const Quat& a, const Quat& b, const float t) { return a.Slerp(b, t); }
[[nodiscard]] inline float Dot(const Quat& a, const Quat& b)            { return a.Dot(b); }

// ─────────────────────────────────────────────────────────────────────────────
//  Matrix
// ─────────────────────────────────────────────────────────────────────────────

[[nodiscard]] inline Mat4 Inverse(const Mat4& m)    { return m.Inversed(); }
[[nodiscard]] inline Mat4 Transpose(const Mat4& m)  { return m.Transposed(); }
[[nodiscard]] inline Mat3 Inverse(const Mat3& m)    { return m.Inversed(); }
[[nodiscard]] inline Mat3 Transpose(const Mat3& m)  { return m.Transposed(); }
[[nodiscard]] inline Mat3 NormalMatrix(const Mat4& model) { return Mat3::NormalMatrix(model); }

// ─────────────────────────────────────────────────────────────────────────────
//  Geometry
// ─────────────────────────────────────────────────────────────────────────────

// Closest point on a line segment to a point
[[nodiscard]] inline Vec3 ClosestPointOnSegment(const Vec3& p, const Vec3& a, const Vec3& b) {
    const Vec3 ab = b - a;
    const float t = Clamp01(Dot(p - a, ab) / ab.LengthSquared());
    return a + ab * t;
}

// Point in triangle (barycentric)
[[nodiscard]] inline bool PointInTriangle(const Vec3& p,
    const Vec3& a, const Vec3& b, const Vec3& c) {
    const Vec3 ab = b - a, ac = c - a, ap = p - a;
    const float d00 = Dot(ab, ab), d01 = Dot(ab, ac);
    const float d11 = Dot(ac, ac), d20 = Dot(ap, ab), d21 = Dot(ap, ac);
    const float denom = d00 * d11 - d01 * d01;
    const float v = (d11 * d20 - d01 * d21) / denom;
    const float w = (d00 * d21 - d01 * d20) / denom;
    return v >= 0.0f && w >= 0.0f && (v + w) <= 1.0f;
}

// Ray-plane intersection — returns distance t along ray, or -1 if no hit
[[nodiscard]] inline float RayPlane(
	const Vec3& rayOrigin, const Vec3& rayDir,
    const Vec3& planePoint, const Vec3& planeNormal
) {
    const float denom = Dot(planeNormal, rayDir);
    if (Abs(denom) < Epsilon) return -1.0f;
    return Dot(planePoint - rayOrigin, planeNormal) / denom;
}

// Ray-sphere intersection — returns distance t along ray, or -1 if no hit
[[nodiscard]] inline float RaySphere(
	const Vec3& rayOrigin, const Vec3& rayDir,
    const Vec3& center, const float radius
) {
    const Vec3 oc = rayOrigin - center;
    const float b = Dot(oc, rayDir);
    const float c = Dot(oc, oc) - radius * radius;
    const float disc = b * b - c;
    if (disc < 0.0f) return -1.0f;
    return -b - Sqrt(disc);
}

// Ray-AABB intersection — returns distance t, or -1 if no hit
[[nodiscard]] inline float RayAABB(
	const Vec3& rayOrigin, const Vec3& rayDir,
    const Vec3& aabbMin, const Vec3& aabbMax
) {
    const auto invDir = Vec3(1.0f / rayDir.x, 1.0f / rayDir.y, 1.0f / rayDir.z);
    const Vec3 t0 = (aabbMin - rayOrigin) * invDir;
    const Vec3 t1 = (aabbMax - rayOrigin) * invDir;
    const Vec3 tMin = Min(t0, t1);
    const Vec3 tMax = Max(t0, t1);
    const float tNear = Max(Max(tMin.x, tMin.y), tMin.z);
    const float tFar  = Min(Min(tMax.x, tMax.y), tMax.z);
    if (tNear > tFar || tFar < 0.0f) return -1.0f;
    return tNear;
}

// AABB overlap test
[[nodiscard]] inline bool AABBOverlap(const Vec3& aMin, const Vec3& aMax,
                                       const Vec3& bMin, const Vec3& bMax) {
    return aMin.x <= bMax.x && aMax.x >= bMin.x
        && aMin.y <= bMax.y && aMax.y >= bMin.y
        && aMin.z <= bMax.z && aMax.z >= bMin.z;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Easing  (t in [0, 1])
// ─────────────────────────────────────────────────────────────────────────────

[[nodiscard]] inline float EaseInQuad(const float t)    { return t * t; }
[[nodiscard]] inline float EaseOutQuad(const float t)   { return 1.0f - (1.0f - t) * (1.0f - t); }
[[nodiscard]] inline float EaseInOutQuad(const float t) { return t < 0.5f ? 2.0f * t * t : 1.0f - Pow(-2.0f * t + 2.0f, 2.0f) * 0.5f; }

[[nodiscard]] inline float EaseInCubic(const float t)    { return t * t * t; }
[[nodiscard]] inline float EaseOutCubic(const float t)   { return 1.0f - Pow(1.0f - t, 3.0f); }
[[nodiscard]] inline float EaseInOutCubic(const float t) { return t < 0.5f ? 4.0f * t * t * t : 1.0f - Pow(-2.0f * t + 2.0f, 3.0f) * 0.5f; }

[[nodiscard]] inline float EaseInQuart(const float t)    { return t * t * t * t; }
[[nodiscard]] inline float EaseOutQuart(const float t)   { return 1.0f - Pow(1.0f - t, 4.0f); }
[[nodiscard]] inline float EaseInOutQuart(const float t) { return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - Pow(-2.0f * t + 2.0f, 4.0f) * 0.5f; }

[[nodiscard]] inline float EaseInSine(const float t)    { return 1.0f - Cos(t * HalfPi); }
[[nodiscard]] inline float EaseOutSine(const float t)   { return Sin(t * HalfPi); }
[[nodiscard]] inline float EaseInOutSine(const float t) { return -(Cos(Pi * t) - 1.0f) * 0.5f; }

[[nodiscard]] inline float EaseInExpo(const float t)    { return t < Epsilon ? 0.0f : Pow(2.0f, 10.0f * t - 10.0f); }
[[nodiscard]] inline float EaseOutExpo(const float t)   { return IsApproximately(t, 1.0f) ? 1.0f : 1.0f - Pow(2.0f, -10.0f * t); }
[[nodiscard]] inline float EaseInOutExpo(const float t) {
    if (t < Epsilon) return 0.0f;
    if (IsApproximately(t, 1.0f)) return 1.0f;
    return t < 0.5f
        ? Pow(2.0f, 20.0f * t - 10.0f) * 0.5f
        : (2.0f - Pow(2.0f, -20.0f * t + 10.0f)) * 0.5f;
}

[[nodiscard]] inline float EaseInElastic(const float t) {
    if (t < Epsilon) return 0.0f;
    if (IsApproximately(t, 1.0f)) return 1.0f;
    return -Pow(2.0f, 10.0f * t - 10.0f) * Sin((t * 10.0f - 10.75f) * (Tau / 3.0f));
}

[[nodiscard]] inline float EaseOutElastic(const float t) {
    if (t < Epsilon) return 0.0f;
    if (IsApproximately(t, 1.0f)) return 1.0f;
    return Pow(2.0f, -10.0f * t) * Sin((t * 10.0f - 0.75f) * (Tau / 3.0f)) + 1.0f;
}

[[nodiscard]] inline float EaseOutBounce(float t) {
    constexpr float n1 = 7.5625f, d1 = 2.75f;
    if (t < 1.0f / d1) { return n1 * t * t; }
    if (t < 2.0f / d1) { t -= 1.5f   / d1; return n1 * t * t + 0.75f; }
    if (t < 2.5f / d1) { t -= 2.25f  / d1; return n1 * t * t + 0.9375; }
					   { t -= 2.625f / d1; return n1 * t * t + 0.984375f; }
}

[[nodiscard]] inline float EaseInBounce(const float t) { return 1.0f - EaseOutBounce(1.0f - t); }

} // namespace Engine

