#pragma once

#include "../../utils/public/Singleton.h"
#include "engine/core/math/public/Vec2.h"
#include "engine/utils/public/ReactiveCommand.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <string>

namespace Engine::Core {

class Window : public Utils::Singleton<Window> {
	friend class Singleton;

public:
	Utils::ReactiveVoidCommand OnWindowResized{};

	Window() = default;
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	~Window();

	[[nodiscard]] bool Initialize(int width, int height, std::string_view title);

	void ToggleFullscreen();
	void SetIcons() const;
	void SwapBuffers() const;
	static void SetVSync(bool enabled);
	static void PollEvents();

	[[nodiscard]] bool IsOpen() const;
	[[nodiscard]] float GetAspectRatio() const;
	[[nodiscard]] constexpr Vec2 GetResolution() const noexcept { return Vec2(m_Width, m_Height); }
	[[nodiscard]] constexpr int GetWidth() const noexcept { return m_Width; }
	[[nodiscard]] constexpr int GetHeight() const noexcept { return m_Height; }
	[[nodiscard]] constexpr GLFWwindow* GetHandle() const noexcept { return m_Handle; }

private:
	void SetupGLState() const;

private:
	GLFWwindow* m_Handle { nullptr };
	int m_Width { 1280 };
	int m_Height { 720 };
	int m_WindowedX { 100 };
	int m_WindowedY { 100 };
	int m_WindowedWidth { 1280 };
	int m_WindowedHeight { 720 };
	bool m_IsFullscreen { false };
	std::string_view m_Title{};
};

} // namespace Core