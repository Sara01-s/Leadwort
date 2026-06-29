#pragma once

#include "engine/core/math/public/Math.h"

#include <algorithm>
#include <glm/glm.hpp>

struct Vec2;
struct Vec4;

namespace Engine {

struct Color {
    float r, g, b, a;

    explicit constexpr Color(const float r, const float g, const float b, const float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    // glm interop
    explicit constexpr Color(const Vec4& v) : r(v.x), g(v.y), b(v.z), a(v.w) {}
    explicit constexpr Color(const Vec3& v, const float a = 1.0f) : r(v.x), g(v.y), b(v.z), a(a) {}
    explicit constexpr operator Vec4() const { return Vec4(r, g, b, a); }
    explicit constexpr operator Vec3() const { return Vec3(r, g, b ); }

    bool operator==(const Color&) const = default;

    // Arithmetic
    constexpr Color operator+(const Color& o) const { return Color(r + o.r, g + o.g, b + o.b, a + o.a); }
    constexpr Color operator-(const Color& o) const { return Color(r - o.r, g - o.g, b - o.b, a - o.a); }
    constexpr Color operator*(const Color& o) const { return Color(r * o.r, g * o.g, b * o.b, a * o.a); }
    constexpr Color operator*(const float s)  const { return Color(r * s,   g * s,   b * s,   a * s  ); }
    constexpr Color operator/(const float s)  const { return Color(r / s,   g / s,   b / s,   a / s  ); }

    constexpr Color& operator+=(const Color& o) { r += o.r; g += o.g; b += o.b; a += o.a; return *this; }
    constexpr Color& operator-=(const Color& o) { r -= o.r; g -= o.g; b -= o.b; a -= o.a; return *this; }
    constexpr Color& operator*=(const float s)  { r *= s;   g *= s;   b *= s;   a *= s;   return *this; }
    constexpr Color& operator/=(const float s)  { r /= s;   g /= s;   b /= s;   a /= s;   return *this; }

    // Utilities
    [[nodiscard]] constexpr Color WithAlpha(const float newAlpha) const { return Color(r, g, b, newAlpha ); }
    [[nodiscard]] constexpr Color WithR(const float newR)         const { return Color(newR, g, b, a); }
    [[nodiscard]] constexpr Color WithG(const float newG)         const { return Color(r, newG, b, a); }
    [[nodiscard]] constexpr Color WithB(const float newB)         const { return Color(r, g, newB, a); }

	[[nodiscard]] static Color Lerp(const Color& a, const Color& b, const float t) {
    	return Color {
    		glm::mix(a.r, b.r, t),
			glm::mix(a.g, b.g, t),
			glm::mix(a.b, b.b, t),
			glm::mix(a.a, b.a, t)
		};
    }

    [[nodiscard]] Color Clamped() const {
        return Color {
            std::clamp(r, 0.0f, 1.0f),
            std::clamp(g, 0.0f, 1.0f),
            std::clamp(b, 0.0f, 1.0f),
            std::clamp(a, 0.0f, 1.0f)
        };
    }

    [[nodiscard]] constexpr Color ToLinear() const {
        return Color(r * r, g * g, b * b, a);
    }

    [[nodiscard]] Color ToGamma() const {
        return Color(std::sqrt(r), std::sqrt(g), std::sqrt(b), a);
    }

	// Luminance (perceptual)
	static constexpr auto s_Luma = Vec3(0.2126f, 0.7152f, 0.0722f);
	[[nodiscard]] float Luminance() const { return s_Luma.Dot(Vec3(r, g, b)); }

    // From 0–255 integers
    static constexpr Color FromRGB8(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) {
        constexpr float inv = 1.0f / 255.0f;
        return Color(r * inv, g * inv, b * inv, a * inv);
    }

    // From 0xRRGGBBAA hex
    static constexpr Color FromHex(const uint32_t hex) {
        return FromRGB8(
            (hex >> 24) & 0xFF,
            (hex >> 16) & 0xFF,
            (hex >>  8) & 0xFF,
            (hex >>  0) & 0xFF
        );
    }

    // ── Basic ────────────────────────────────────────────────────────────
    static constexpr Color White()   { return Color(1.0f, 1.0f, 1.0f); }
    static constexpr Color Black()   { return Color(0.0f, 0.0f, 0.0f); }
    static constexpr Color Red()     { return Color(1.0f, 0.0f, 0.0f); }
    static constexpr Color Green()   { return Color(0.0f, 1.0f, 0.0f); }
    static constexpr Color Blue()    { return Color(0.0f, 0.0f, 1.0f); }
    static constexpr Color Cyan()    { return Color(0.0f, 1.0f, 1.0f); }
    static constexpr Color Magenta() { return Color(1.0f, 0.0f, 1.0f); }
    static constexpr Color Yellow()  { return Color(1.0f, 1.0f, 0.0f); }

    // ── Transparency ─────────────────────────────────────────────────────
    static constexpr Color Transparent()     { return Color(0.0f, 0.0f, 0.0f, 0.0f); }
    static constexpr Color HalfTransparent() { return Color(0.0f, 0.0f, 0.0f, 0.5f); }

    // ── Grayscale ────────────────────────────────────────────────────────
    static constexpr Color Gray05() { return Color(0.05f, 0.05f, 0.05f); }
    static constexpr Color Gray10() { return Color(0.10f, 0.10f, 0.10f); }
    static constexpr Color Gray15() { return Color(0.15f, 0.15f, 0.15f); }
    static constexpr Color Gray20() { return Color(0.20f, 0.20f, 0.20f); }
    static constexpr Color Gray25() { return Color(0.25f, 0.25f, 0.25f); }
    static constexpr Color Gray30() { return Color(0.30f, 0.30f, 0.30f); }
    static constexpr Color Gray35() { return Color(0.35f, 0.35f, 0.35f); }
    static constexpr Color Gray40() { return Color(0.40f, 0.40f, 0.40f); }
    static constexpr Color Gray45() { return Color(0.45f, 0.45f, 0.45f); }
    static constexpr Color Gray50() { return Color(0.50f, 0.50f, 0.50f); }
    static constexpr Color Gray55() { return Color(0.55f, 0.55f, 0.55f); }
    static constexpr Color Gray60() { return Color(0.60f, 0.60f, 0.60f); }
    static constexpr Color Gray65() { return Color(0.65f, 0.65f, 0.65f); }
    static constexpr Color Gray70() { return Color(0.70f, 0.70f, 0.70f); }
    static constexpr Color Gray75() { return Color(0.75f, 0.75f, 0.75f); }
    static constexpr Color Gray80() { return Color(0.80f, 0.80f, 0.80f); }
    static constexpr Color Gray85() { return Color(0.85f, 0.85f, 0.85f); }
    static constexpr Color Gray90() { return Color(0.90f, 0.90f, 0.90f); }
    static constexpr Color Gray95() { return Color(0.95f, 0.95f, 0.95f); }

    // ── Reds ─────────────────────────────────────────────────────────────
    static constexpr Color DarkRed()    { return Color(0.55f, 0.00f, 0.00f); }
    static constexpr Color Crimson()    { return Color(0.86f, 0.08f, 0.24f); }
    static constexpr Color Scarlet()    { return Color(1.00f, 0.14f, 0.00f); }
    static constexpr Color Salmon()     { return Color(0.98f, 0.50f, 0.45f); }
    static constexpr Color Coral()      { return Color(1.00f, 0.50f, 0.31f); }
    static constexpr Color Tomato()     { return Color(1.00f, 0.39f, 0.28f); }
    static constexpr Color IndianRed()  { return Color(0.80f, 0.36f, 0.36f); }

    // ── Oranges ──────────────────────────────────────────────────────────
    static constexpr Color Orange()     { return Color(1.00f, 0.65f, 0.00f); }
    static constexpr Color DarkOrange() { return Color(1.00f, 0.55f, 0.00f); }
    static constexpr Color Amber()      { return Color(1.00f, 0.75f, 0.00f); }
    static constexpr Color Pumpkin()    { return Color(1.00f, 0.46f, 0.09f); }

    // ── Yellows ──────────────────────────────────────────────────────────
    static constexpr Color Gold()   { return Color(1.00f, 0.84f, 0.00f); }
    static constexpr Color Lemon()  { return Color(1.00f, 0.97f, 0.00f); }
    static constexpr Color Khaki()  { return Color(0.94f, 0.90f, 0.55f); }

    // ── Greens ───────────────────────────────────────────────────────────
    static constexpr Color DarkGreen()   { return Color(0.00f, 0.39f, 0.00f); }
    static constexpr Color Lime()        { return Color(0.75f, 1.00f, 0.00f); }
    static constexpr Color ForestGreen() { return Color(0.13f, 0.55f, 0.13f); }
    static constexpr Color Emerald()     { return Color(0.31f, 0.78f, 0.47f); }
    static constexpr Color Mint()        { return Color(0.60f, 1.00f, 0.60f); }
    static constexpr Color Olive()       { return Color(0.50f, 0.50f, 0.00f); }
    static constexpr Color SeaGreen()    { return Color(0.18f, 0.55f, 0.34f); }

    // ── Cyans / Teals ────────────────────────────────────────────────────
    static constexpr Color Turquoise() { return Color(0.25f, 0.88f, 0.82f); }
    static constexpr Color Teal()      { return Color(0.00f, 0.50f, 0.50f); }
    static constexpr Color Aqua()      { return Color(0.00f, 1.00f, 1.00f); }

    // ── Blues ────────────────────────────────────────────────────────────
    static constexpr Color Navy()         { return Color(0.00f, 0.00f, 0.50f); }
    static constexpr Color RoyalBlue()    { return Color(0.25f, 0.41f, 0.88f); }
    static constexpr Color SkyBlue()      { return Color(0.53f, 0.81f, 0.92f); }
    static constexpr Color DeepSkyBlue()  { return Color(0.00f, 0.75f, 1.00f); }
    static constexpr Color DodgerBlue()   { return Color(0.12f, 0.56f, 1.00f); }
    static constexpr Color SteelBlue()    { return Color(0.27f, 0.51f, 0.71f); }
    static constexpr Color MidnightBlue() { return Color(0.10f, 0.10f, 0.44f); }

    // ── Purples ──────────────────────────────────────────────────────────
    static constexpr Color Purple()   { return Color(0.50f, 0.00f, 0.50f); }
    static constexpr Color Violet()   { return Color(0.93f, 0.51f, 0.93f); }
    static constexpr Color Indigo()   { return Color(0.29f, 0.00f, 0.51f); }
    static constexpr Color Lavender() { return Color(0.90f, 0.90f, 0.98f); }
    static constexpr Color Plum()     { return Color(0.87f, 0.63f, 0.87f); }
    static constexpr Color Orchid()   { return Color(0.85f, 0.44f, 0.84f); }

    // ── Pinks ────────────────────────────────────────────────────────────
    static constexpr Color Pink()      { return Color(1.00f, 0.75f, 0.80f); }
    static constexpr Color HotPink()   { return Color(1.00f, 0.41f, 0.71f); }
    static constexpr Color DeepPink()  { return Color(1.00f, 0.08f, 0.58f); }
    static constexpr Color Rose()      { return Color(1.00f, 0.00f, 0.50f); }

    // ── Browns ───────────────────────────────────────────────────────────
    static constexpr Color Brown()     { return Color(0.65f, 0.16f, 0.16f); }
    static constexpr Color Chocolate() { return Color(0.82f, 0.41f, 0.12f); }
    static constexpr Color Sienna()    { return Color(0.63f, 0.32f, 0.18f); }
    static constexpr Color Tan()       { return Color(0.82f, 0.71f, 0.55f); }
    static constexpr Color Beige()     { return Color(0.96f, 0.96f, 0.86f); }

    // ── Special / UI ─────────────────────────────────────────────────────
    static constexpr Color CoolPurple()     { return Color(0.0667f, 0.00f, 0.0902f); }
    static constexpr Color DiscordBlurple() { return Color(0.35f,   0.39f, 0.85f);   }
    static constexpr Color Warning()        { return Color(1.00f,   0.75f, 0.00f);   }
    static constexpr Color Error()          { return Color(0.90f,   0.20f, 0.20f);   }
    static constexpr Color Success()        { return Color(0.20f,   0.80f, 0.20f);   }
    static constexpr Color Info()           { return Color(0.20f,   0.60f, 1.00f);   }
};

inline Color operator*(const float s, const Color& c) { return c * s; }

} // namespace Engine::Utils