#include <Leadwort/systems/public/Input.h>
#include <Leadwort/utils/public/Logger.h>

#include <cmath>

namespace Leadwort::Systems {

void Input::Init(GLFWwindow* window) {
	s_Window = window;

	glfwSetScrollCallback(s_Window, [](GLFWwindow*, const double xOffset, const double yOffset) {
		Mouse::OnScroll(static_cast<float>(xOffset), static_cast<float>(yOffset));
	});
}

void Input::Update(const float delta) {
	LW_ASSERT(s_Window, "Input uninitialized, please call Input::Init(GLFWindow*)");

	Mouse::ClearDeltas();
	Mouse::Update();

	Keyboard::Update(delta);
}

// Axis
float Input::GetAxis(const Player player, const Axis axis) {
	const auto& a = s_Axes[static_cast<int>(player)];
	return axis == Axis::Horizontal ? a.x : a.y;
}

Vec2 Input::GetAxis(const Player player) {
	return s_Axes[static_cast<int>(player)];
}

// Keyboard
bool Input::Keyboard::IsPressed(const Key key) {
	return s_CurrentKeys[static_cast<int>(key)];
}

bool Input::Keyboard::IsJustPressed(const Key key) {
	return s_CurrentKeys[static_cast<int>(key)] && !s_PreviousKeys[static_cast<int>(key)];
}

bool Input::Keyboard::IsJustReleased(const Key key) {
	return !s_CurrentKeys[static_cast<int>(key)] && s_PreviousKeys[static_cast<int>(key)];
}

bool Input::Keyboard::IsOrderedCombo(const Key first, const Key second) {
	const int f = static_cast<int>(first);
	const int s = static_cast<int>(second);

	if (!IsPressed(first) || !IsPressed(second)) {
		return false;
	}

	return IsJustPressed(second) && s_PressFrame[f] < s_PressFrame[s];
}

void Input::Keyboard::Update(const float delta) {
	s_PreviousKeys = s_CurrentKeys;

	for (int i = 0; i <= KeyLast; ++i) {
		const bool wasPressed = s_CurrentKeys[i];
		const bool isPressed = glfwGetKey(s_Window, i) == GLFW_PRESS;

		s_CurrentKeys[i] = isPressed;

		if (isPressed && !wasPressed) {
			s_PressFrame[i] = s_FrameCount;
		}
	}
	++s_FrameCount;

	for (int p = 0; p < PlayerCount; ++p) {
		const auto& [up, down, left, right] = s_PlayerKeys[p];
		auto& a = s_Axes[p];
		a.x = SmoothAxis(a.x, delta, left, right);
		a.y = SmoothAxis(a.y, delta, down, up);
	}
}

// Mouse
Vec2 Input::Mouse::GetPosition() {
	return s_MousePosition;
}

Vec2 Input::Mouse::GetDelta() {
	return s_MouseDelta;
}

Vec2 Input::Mouse::GetScroll() {
	return s_MouseScroll;
}

bool Input::Mouse::IsCaptured() {
	return s_MouseCaptured;
}

bool Input::Mouse::IsButtonPressed(const Button button) {
	LW_ASSERT(s_Window, "Input uninitialized, please call Input::Init(GLFWindow*)");
	return glfwGetMouseButton(s_Window, static_cast<int>(button)) == GLFW_PRESS;
}

void Input::Mouse::SetCaptured(const bool captured) {
	LW_ASSERT(s_Window, "Input uninitialized, please call Input::Init(GLFWindow*)");

	s_MouseCaptured = captured;
	const int mode = captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;

	glfwSetInputMode(s_Window, GLFW_CURSOR, mode);

	s_MouseFirstFrame = true;
}

bool Input::Mouse::IsButtonJustPressed(const Button button) {
	const int b = static_cast<int>(button);
	return s_CurrentMouseButtons[b] && !s_PreviousMouseButtons[b];
}

bool Input::Mouse::IsButtonJustReleased(const Button button) {
	const int b = static_cast<int>(button);
	return !s_CurrentMouseButtons[b] && s_PreviousMouseButtons[b];
}

void Input::Mouse::Update() {
	LW_ASSERT(s_Window, "Input uninitialized, please call Input::Init(GLFWindow*)");

	s_PreviousMouseButtons = s_CurrentMouseButtons;
	for (int b = 0; b < MouseButtonCount; ++b) {
		s_CurrentMouseButtons[b] = glfwGetMouseButton(s_Window, b) == GLFW_PRESS;
	}

	double x{}, y{};
	glfwGetCursorPos(s_Window, &x, &y);

	s_MousePosition = Vec2(static_cast<float>(x), static_cast<float>(y));

	if (s_MouseFirstFrame) {
		s_MouseLastPosition = s_MousePosition;
		s_MouseFirstFrame = false;
	}

	s_MouseDelta = s_MousePosition - s_MouseLastPosition;
	s_MouseLastPosition = s_MousePosition;
}

void Input::Mouse::ClearDeltas() {
	s_MouseDelta = Vec2::Zero();
	s_MouseScroll = Vec2::Zero();
}

void Input::Mouse::OnScroll(const float xOffset, const float yOffset) {
	s_MouseScroll.x += xOffset;
	s_MouseScroll.y += yOffset;
}

void Input::Mouse::Reset() {
	SetCaptured(false);

	s_MouseDelta = Vec2::Zero();
	s_MouseScroll = Vec2::Zero();
	s_MouseFirstFrame = true;

	s_CurrentMouseButtons.fill(false);
	s_PreviousMouseButtons.fill(false);
}

// Clear
void Input::Clear() {
	s_CurrentKeys.fill(false);
	s_PreviousKeys.fill(false);

	for (auto& axis: s_Axes) {
		axis = Vec2::Zero();
	}

	Mouse::Reset();
}

// Internals
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