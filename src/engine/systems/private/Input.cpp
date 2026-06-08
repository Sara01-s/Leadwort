#include "engine/systems/public/Input.h"

#include "engine/utils/public/Logger.h"

#include <cmath>

namespace Engine::Systems {

// ── Static storage ────────────────────────────────────────────────────────────

GLFWwindow* Input::s_Window = nullptr;
std::array<bool, GLFW_KEY_LAST + 1> Input::s_CurrentKeys = {};
std::array<bool, GLFW_KEY_LAST + 1> Input::s_PreviousKeys = {};
std::array<glm::vec2, Input::PlayerCount> Input::s_Axes = {};

std::array<Input::KeyMap, Input::PlayerCount> Input::s_PlayerKeys = {{
	{Key::W, Key::S, Key::A, Key::D},			 // P1
	{Key::Up, Key::Down, Key::Left, Key::Right}, // P2
}};

glm::vec2 Input::s_MousePosition = {};
glm::vec2 Input::s_MouseDelta = {};
glm::vec2 Input::s_MouseLastPosition = {};
glm::vec2 Input::s_MouseScroll = {};
bool Input::s_MouseCaptured = false;
bool Input::s_MouseFirstFrame = true;

// ── Init ──────────────────────────────────────────────────────────────────────

void Input::Init(GLFWwindow* window) {
	s_Window = window;

	glfwSetScrollCallback(s_Window, [](GLFWwindow*, const double xOffset, const double yOffset) {
		Mouse::OnScroll(static_cast<float>(xOffset), static_cast<float>(yOffset));
	});
}

// ── Update ────────────────────────────────────────────────────────────────────

void Input::Update(const float delta) {
	CORE_ASSERT(s_Window, "Input uninitialized, please call Input::Init(GLFWindow*)");

	Mouse::ClearDeltas();
	Mouse::Update();

	s_PreviousKeys = s_CurrentKeys;

	for (int i = 0; i <= KeyLast; ++i) {
		s_CurrentKeys[i] = glfwGetKey(s_Window, i) == GLFW_PRESS;
	}

	for (int p = 0; p < PlayerCount; ++p) {
		const auto& [up, down, left, right] = s_PlayerKeys[p];
		auto& a = s_Axes[p];
		a.x = SmoothAxis(a.x, delta, left, right);
		a.y = SmoothAxis(a.y, delta, down, up);
	}
}

// ── Axis ──────────────────────────────────────────────────────────────────────

float Input::GetAxis(const Player player, const Axis axis) {
	const auto& a = s_Axes[static_cast<int>(player)];
	return axis == Axis::Horizontal ? a.x : a.y;
}

glm::vec2 Input::GetAxis(const Player player) {
	return s_Axes[static_cast<int>(player)];
}

// ── Keyboard ──────────────────────────────────────────────────────────────────

bool Input::Keyboard::IsPressed(const Key key) {
	return s_CurrentKeys[static_cast<int>(key)];
}

bool Input::Keyboard::IsJustPressed(const Key key) {
	return s_CurrentKeys[static_cast<int>(key)] && !s_PreviousKeys[static_cast<int>(key)];
}

bool Input::Keyboard::IsJustReleased(const Key key) {
	return !s_CurrentKeys[static_cast<int>(key)] && s_PreviousKeys[static_cast<int>(key)];
}

// ── Mouse ─────────────────────────────────────────────────────────────────────

glm::vec2 Input::Mouse::GetPosition() {
	return s_MousePosition;
}

glm::vec2 Input::Mouse::GetDelta() {
	return s_MouseDelta;
}

glm::vec2 Input::Mouse::GetScroll() {
	return s_MouseScroll;
}

bool Input::Mouse::IsCaptured() {
	return s_MouseCaptured;
}

bool Input::Mouse::IsButtonPressed(const Button button) {
	return IsButtonPressed(static_cast<int>(button));
}

bool Input::Mouse::IsButtonPressed(const int button) {
	CORE_ASSERT(s_Window, "Input uninitialized, please call Input::Init(GLFWindow*)");
	return glfwGetMouseButton(s_Window, button) == GLFW_PRESS;
}

void Input::Mouse::SetCaptured(const bool captured) {
	CORE_ASSERT(s_Window, "Input uninitialized, please call Input::Init(GLFWindow*)");

	s_MouseCaptured = captured;
	const int mode = captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;

	glfwSetInputMode(s_Window, GLFW_CURSOR, mode);

	s_MouseFirstFrame = true;
}

void Input::Mouse::Update() {
	CORE_ASSERT(s_Window, "Input uninitialized, please call Input::Init(GLFWindow*)");

	double x, y;
	glfwGetCursorPos(s_Window, &x, &y);

	s_MousePosition = { static_cast<float>(x), static_cast<float>(y) };

	if (s_MouseFirstFrame) {
		s_MouseLastPosition = s_MousePosition;
		s_MouseFirstFrame = false;
	}

	s_MouseDelta = s_MousePosition - s_MouseLastPosition;
	s_MouseLastPosition = s_MousePosition;
}

void Input::Mouse::ClearDeltas() {
	s_MouseDelta = {};
	s_MouseScroll = {};
}

void Input::Mouse::OnScroll(const float xOffset, const float yOffset) {
	s_MouseScroll.x += xOffset;
	s_MouseScroll.y += yOffset;
}

void Input::Mouse::Reset() {
	SetCaptured(false);
	s_MouseDelta = {};
	s_MouseScroll = {};
	s_MouseFirstFrame = true;
}

// ── Clear ─────────────────────────────────────────────────────────────────────

void Input::Clear() {
	s_CurrentKeys.fill(false);
	s_PreviousKeys.fill(false);

	for (auto& axis: s_Axes) {
		axis = {};
	}

	Mouse::Reset();
}

// ── Internals ─────────────────────────────────────────────────────────────────

float Input::MoveToward(const float current, const float target, const float step) {
	const float delta = target - current;
	return std::abs(delta) <= step ? target : current + std::copysign(step, delta);
}

float Input::SmoothAxis(const float current, const float delta, const Key neg, const Key pos) {
	float target = 0.0f;

	if (Keyboard::IsPressed(pos)) target += 1.0f;
	if (Keyboard::IsPressed(neg)) target -= 1.0f;

	return MoveToward(current, target, SmoothSpeed * delta);
}

} // namespace Engine::Systems