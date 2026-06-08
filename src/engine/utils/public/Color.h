#pragma once

namespace Engine::Utils {

struct Color {
    float r, g, b, a;

    constexpr Color(const float r, const float g, const float b, const float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    bool operator==(const Color&) const = default;

    // ── Basic ────────────────────────────────────────────────────────────
    static constexpr Color White()   { return {1.0f, 1.0f, 1.0f}; }
    static constexpr Color Black()   { return {0.0f, 0.0f, 0.0f}; }
    static constexpr Color Red()     { return {1.0f, 0.0f, 0.0f}; }
    static constexpr Color Green()   { return {0.0f, 1.0f, 0.0f}; }
    static constexpr Color Blue()    { return {0.0f, 0.0f, 1.0f}; }
    static constexpr Color Cyan()    { return {0.0f, 1.0f, 1.0f}; }
    static constexpr Color Magenta() { return {1.0f, 0.0f, 1.0f}; }
    static constexpr Color Yellow()  { return {1.0f, 1.0f, 0.0f}; }

    // ── Transparency ─────────────────────────────────────────────────────
    static constexpr Color Transparent()     { return {0.0f, 0.0f, 0.0f, 0.0f}; }
    static constexpr Color HalfTransparent() { return {0.0f, 0.0f, 0.0f, 0.5f}; }

    // ── Grayscale ────────────────────────────────────────────────────────
    static constexpr Color Gray05() { return {0.05f, 0.05f, 0.05f}; }
    static constexpr Color Gray10() { return {0.10f, 0.10f, 0.10f}; }
    static constexpr Color Gray15() { return {0.15f, 0.15f, 0.15f}; }
    static constexpr Color Gray20() { return {0.20f, 0.20f, 0.20f}; }
    static constexpr Color Gray25() { return {0.25f, 0.25f, 0.25f}; }
    static constexpr Color Gray30() { return {0.30f, 0.30f, 0.30f}; }
    static constexpr Color Gray35() { return {0.35f, 0.35f, 0.35f}; }
    static constexpr Color Gray40() { return {0.40f, 0.40f, 0.40f}; }
    static constexpr Color Gray45() { return {0.45f, 0.45f, 0.45f}; }
    static constexpr Color Gray50() { return {0.50f, 0.50f, 0.50f}; }
    static constexpr Color Gray55() { return {0.55f, 0.55f, 0.55f}; }
    static constexpr Color Gray60() { return {0.60f, 0.60f, 0.60f}; }
    static constexpr Color Gray65() { return {0.65f, 0.65f, 0.65f}; }
    static constexpr Color Gray70() { return {0.70f, 0.70f, 0.70f}; }
    static constexpr Color Gray75() { return {0.75f, 0.75f, 0.75f}; }
    static constexpr Color Gray80() { return {0.80f, 0.80f, 0.80f}; }
    static constexpr Color Gray85() { return {0.85f, 0.85f, 0.85f}; }
    static constexpr Color Gray90() { return {0.90f, 0.90f, 0.90f}; }
    static constexpr Color Gray95() { return {0.95f, 0.95f, 0.95f}; }

    // ── Reds ─────────────────────────────────────────────────────────────
    static constexpr Color DarkRed()    { return {0.55f, 0.00f, 0.00f}; }
    static constexpr Color Crimson()    { return {0.86f, 0.08f, 0.24f}; }
    static constexpr Color Scarlet()    { return {1.00f, 0.14f, 0.00f}; }
    static constexpr Color Salmon()     { return {0.98f, 0.50f, 0.45f}; }
    static constexpr Color Coral()      { return {1.00f, 0.50f, 0.31f}; }
    static constexpr Color Tomato()     { return {1.00f, 0.39f, 0.28f}; }
    static constexpr Color IndianRed()  { return {0.80f, 0.36f, 0.36f}; }

    // ── Oranges ──────────────────────────────────────────────────────────
    static constexpr Color Orange()     { return {1.00f, 0.65f, 0.00f}; }
    static constexpr Color DarkOrange() { return {1.00f, 0.55f, 0.00f}; }
    static constexpr Color Amber()      { return {1.00f, 0.75f, 0.00f}; }
    static constexpr Color Pumpkin()    { return {1.00f, 0.46f, 0.09f}; }

    // ── Yellows ──────────────────────────────────────────────────────────
    static constexpr Color Gold()   { return {1.00f, 0.84f, 0.00f}; }
    static constexpr Color Lemon()  { return {1.00f, 0.97f, 0.00f}; }
    static constexpr Color Khaki()  { return {0.94f, 0.90f, 0.55f}; }

    // ── Greens ───────────────────────────────────────────────────────────
    static constexpr Color DarkGreen()   { return {0.00f, 0.39f, 0.00f}; }
    static constexpr Color Lime()        { return {0.75f, 1.00f, 0.00f}; }
    static constexpr Color ForestGreen() { return {0.13f, 0.55f, 0.13f}; }
    static constexpr Color Emerald()     { return {0.31f, 0.78f, 0.47f}; }
    static constexpr Color Mint()        { return {0.60f, 1.00f, 0.60f}; }
    static constexpr Color Olive()       { return {0.50f, 0.50f, 0.00f}; }
    static constexpr Color SeaGreen()    { return {0.18f, 0.55f, 0.34f}; }

    // ── Cyans / Teals ────────────────────────────────────────────────────
    static constexpr Color Turquoise() { return {0.25f, 0.88f, 0.82f}; }
    static constexpr Color Teal()      { return {0.00f, 0.50f, 0.50f}; }
    static constexpr Color Aqua()      { return {0.00f, 1.00f, 1.00f}; }

    // ── Blues ────────────────────────────────────────────────────────────
    static constexpr Color Navy()         { return {0.00f, 0.00f, 0.50f}; }
    static constexpr Color RoyalBlue()    { return {0.25f, 0.41f, 0.88f}; }
    static constexpr Color SkyBlue()      { return {0.53f, 0.81f, 0.92f}; }
    static constexpr Color DeepSkyBlue()  { return {0.00f, 0.75f, 1.00f}; }
    static constexpr Color DodgerBlue()   { return {0.12f, 0.56f, 1.00f}; }
    static constexpr Color SteelBlue()    { return {0.27f, 0.51f, 0.71f}; }
    static constexpr Color MidnightBlue() { return {0.10f, 0.10f, 0.44f}; }

    // ── Purples ──────────────────────────────────────────────────────────
    static constexpr Color Purple()   { return {0.50f, 0.00f, 0.50f}; }
    static constexpr Color Violet()   { return {0.93f, 0.51f, 0.93f}; }
    static constexpr Color Indigo()   { return {0.29f, 0.00f, 0.51f}; }
    static constexpr Color Lavender() { return {0.90f, 0.90f, 0.98f}; }
    static constexpr Color Plum()     { return {0.87f, 0.63f, 0.87f}; }
    static constexpr Color Orchid()   { return {0.85f, 0.44f, 0.84f}; }

    // ── Pinks ────────────────────────────────────────────────────────────
    static constexpr Color Pink()      { return {1.00f, 0.75f, 0.80f}; }
    static constexpr Color HotPink()   { return {1.00f, 0.41f, 0.71f}; }
    static constexpr Color DeepPink()  { return {1.00f, 0.08f, 0.58f}; }
    static constexpr Color Rose()      { return {1.00f, 0.00f, 0.50f}; }

    // ── Browns ───────────────────────────────────────────────────────────
    static constexpr Color Brown()     { return {0.65f, 0.16f, 0.16f}; }
    static constexpr Color Chocolate() { return {0.82f, 0.41f, 0.12f}; }
    static constexpr Color Sienna()    { return {0.63f, 0.32f, 0.18f}; }
    static constexpr Color Tan()       { return {0.82f, 0.71f, 0.55f}; }
    static constexpr Color Beige()     { return {0.96f, 0.96f, 0.86f}; }

    // ── Special / UI ─────────────────────────────────────────────────────
    static constexpr Color CoolPurple()     { return {0.0667f, 0.00f, 0.0902f}; }
    static constexpr Color DiscordBlurple() { return {0.35f,   0.39f, 0.85f};   }
    static constexpr Color Warning()        { return {1.00f,   0.75f, 0.00f};   }
    static constexpr Color Error()          { return {0.90f,   0.20f, 0.20f};   }
    static constexpr Color Success()        { return {0.20f,   0.80f, 0.20f};   }
    static constexpr Color Info()           { return {0.20f,   0.60f, 1.00f};   }
};

} // namespace Engine::Utils