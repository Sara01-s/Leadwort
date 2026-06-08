#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <array>

namespace Engine::Systems {

// ── Key ───────────────────────────────────────────────────────────────────────

enum class Key : int {
    // ── Alphanumeric ──────────────────────────────────────────────────────────
    Alpha0 = GLFW_KEY_0, Alpha1 = GLFW_KEY_1, Alpha2 = GLFW_KEY_2, Alpha3 = GLFW_KEY_3,
    Alpha4 = GLFW_KEY_4, Alpha5 = GLFW_KEY_5, Alpha6 = GLFW_KEY_6, Alpha7 = GLFW_KEY_7,
    Alpha8 = GLFW_KEY_8, Alpha9 = GLFW_KEY_9,

    A = GLFW_KEY_A, B = GLFW_KEY_B, C = GLFW_KEY_C, D = GLFW_KEY_D, E = GLFW_KEY_E,
    F = GLFW_KEY_F, G = GLFW_KEY_G, H = GLFW_KEY_H, I = GLFW_KEY_I, J = GLFW_KEY_J,
    K = GLFW_KEY_K, L = GLFW_KEY_L, M = GLFW_KEY_M, N = GLFW_KEY_N, O = GLFW_KEY_O,
    P = GLFW_KEY_P, Q = GLFW_KEY_Q, R = GLFW_KEY_R, S = GLFW_KEY_S, T = GLFW_KEY_T,
    U = GLFW_KEY_U, V = GLFW_KEY_V, W = GLFW_KEY_W, X = GLFW_KEY_X, Y = GLFW_KEY_Y,
    Z = GLFW_KEY_Z,

    // ── Signs and Punctuation ─────────────────────────────────────────────────
    Space        = GLFW_KEY_SPACE,        Apostrophe = GLFW_KEY_APOSTROPHE,
    Comma        = GLFW_KEY_COMMA,        Minus      = GLFW_KEY_MINUS,
    Period       = GLFW_KEY_PERIOD,       Slash      = GLFW_KEY_SLASH,
    Semicolon    = GLFW_KEY_SEMICOLON,    Equal      = GLFW_KEY_EQUAL,
    LeftBracket  = GLFW_KEY_LEFT_BRACKET, Backslash  = GLFW_KEY_BACKSLASH,
    RightBracket = GLFW_KEY_RIGHT_BRACKET,GraveAccent= GLFW_KEY_GRAVE_ACCENT,
    World1       = GLFW_KEY_WORLD_1,      World2     = GLFW_KEY_WORLD_2,

    // ── Navigation and Editing ────────────────────────────────────────────────
    Escape    = GLFW_KEY_ESCAPE, Enter  = GLFW_KEY_ENTER,  Tab    = GLFW_KEY_TAB,
    Backspace = GLFW_KEY_BACKSPACE, Insert = GLFW_KEY_INSERT, Delete = GLFW_KEY_DELETE,
    PageUp    = GLFW_KEY_PAGE_UP,   PageDown = GLFW_KEY_PAGE_DOWN,
    Home      = GLFW_KEY_HOME,      End    = GLFW_KEY_END,

    // ── Directional Arrows ────────────────────────────────────────────────────
    Right = GLFW_KEY_RIGHT, Left = GLFW_KEY_LEFT, Down = GLFW_KEY_DOWN, Up = GLFW_KEY_UP,

    // ── Modifiers and System ──────────────────────────────────────────────────
    LeftShift   = GLFW_KEY_LEFT_SHIFT,   LeftControl = GLFW_KEY_LEFT_CONTROL,
    LeftAlt     = GLFW_KEY_LEFT_ALT,     LeftSuper   = GLFW_KEY_LEFT_SUPER,
    RightShift  = GLFW_KEY_RIGHT_SHIFT,  RightControl= GLFW_KEY_RIGHT_CONTROL,
    RightAlt    = GLFW_KEY_RIGHT_ALT,    RightSuper  = GLFW_KEY_RIGHT_SUPER,
    Menu        = GLFW_KEY_MENU,
    CapsLock    = GLFW_KEY_CAPS_LOCK,    ScrollLock  = GLFW_KEY_SCROLL_LOCK,
    NumLock     = GLFW_KEY_NUM_LOCK,     PrintScreen = GLFW_KEY_PRINT_SCREEN,
    Pause       = GLFW_KEY_PAUSE,

    // ── Function Keys ─────────────────────────────────────────────────────────
    F1  = GLFW_KEY_F1,  F2  = GLFW_KEY_F2,  F3  = GLFW_KEY_F3,  F4  = GLFW_KEY_F4,
    F5  = GLFW_KEY_F5,  F6  = GLFW_KEY_F6,  F7  = GLFW_KEY_F7,  F8  = GLFW_KEY_F8,
    F9  = GLFW_KEY_F9,  F10 = GLFW_KEY_F10, F11 = GLFW_KEY_F11, F12 = GLFW_KEY_F12,
    F13 = GLFW_KEY_F13, F14 = GLFW_KEY_F14, F15 = GLFW_KEY_F15, F16 = GLFW_KEY_F16,
    F17 = GLFW_KEY_F17, F18 = GLFW_KEY_F18, F19 = GLFW_KEY_F19, F20 = GLFW_KEY_F20,
    F21 = GLFW_KEY_F21, F22 = GLFW_KEY_F22, F23 = GLFW_KEY_F23, F24 = GLFW_KEY_F24,
    F25 = GLFW_KEY_F25,

    // ── Numpad ────────────────────────────────────────────────────────────────
    NumPad0        = GLFW_KEY_KP_0,        NumPad1   = GLFW_KEY_KP_1,
    NumPad2        = GLFW_KEY_KP_2,        NumPad3   = GLFW_KEY_KP_3,
    NumPad4        = GLFW_KEY_KP_4,        NumPad5   = GLFW_KEY_KP_5,
    NumPad6        = GLFW_KEY_KP_6,        NumPad7   = GLFW_KEY_KP_7,
    NumPad8        = GLFW_KEY_KP_8,        NumPad9   = GLFW_KEY_KP_9,
    NumPadDecimal  = GLFW_KEY_KP_DECIMAL,  NumPadDivide   = GLFW_KEY_KP_DIVIDE,
    NumPadMultiply = GLFW_KEY_KP_MULTIPLY, NumPadSubtract = GLFW_KEY_KP_SUBTRACT,
    NumPadAdd      = GLFW_KEY_KP_ADD,      NumPadEnter    = GLFW_KEY_KP_ENTER,
    NumPadEqual    = GLFW_KEY_KP_EQUAL,
};

// ── Axis / Player ─────────────────────────────────────────────────────────────

enum class Axis   { Horizontal, Vertical };
enum class Player { P1 = 0, P2 = 1 };

// ── Input ─────────────────────────────────────────────────────────────────────

class Input {
public:
    Input() = delete;

    static void Init(GLFWwindow* window);
    static void Update(float delta);
    static void Clear();

    static float     GetAxis(Player player, Axis axis);
    static glm::vec2 GetAxis(Player player);

    // ── Keyboard ──────────────────────────────────────────────────────────────

    struct Keyboard {
        static bool IsPressed(Key key);
        static bool IsJustPressed(Key key);
        static bool IsJustReleased(Key key);
    };

    // ── Mouse ─────────────────────────────────────────────────────────────────

    struct Mouse {
        enum class Button { Left = 0, Right = 1 };

        static glm::vec2 GetPosition();
        static glm::vec2 GetDelta();
        static glm::vec2 GetScroll();

        static bool IsButtonPressed(Button button);
        static bool IsButtonPressed(int button);

        static bool IsCaptured();
        static void SetCaptured(bool captured);

        static void Update();
        static void ClearDeltas();
        static void OnScroll(float xOffset, float yOffset);
        static void Reset();
    };

private:
    struct KeyMap {
        Key up, down, left, right;
    };

    static float MoveToward(float current, float target, float step);
    static float SmoothAxis(float current, float delta, Key neg, Key pos);

    static constexpr int   KeyLast      = GLFW_KEY_LAST;
    static constexpr float SmoothSpeed  = 15.0f;
    static constexpr int   PlayerCount  = 2;

    static GLFWwindow* s_Window;

    static std::array<bool, GLFW_KEY_LAST + 1> s_CurrentKeys;
    static std::array<bool, GLFW_KEY_LAST + 1> s_PreviousKeys;
    static std::array<glm::vec2, PlayerCount> s_Axes;
    static std::array<KeyMap,    PlayerCount> s_PlayerKeys;

    // Mouse state
    static glm::vec2 s_MousePosition;
    static glm::vec2 s_MouseDelta;
    static glm::vec2 s_MouseLastPosition;
    static glm::vec2 s_MouseScroll;
    static bool      s_MouseCaptured;
    static bool      s_MouseFirstFrame;
};

} // namespace Engine::Systems