#pragma once

#include "../../utils/public/Singleton.h"
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

	bool Initialize(int width, int height, std::string_view title);
	void ToggleFullscreen();
	[[nodiscard]] bool IsOpen() const;
	static void SetVSync(bool enabled);
	void SwapBuffers() const;
	static void PollEvents();
	[[nodiscard]] float GetAspectRatio() const;

	int GetWidth() const noexcept { return m_Width; }
	int GetHeight() const noexcept { return m_Height; }
	GLFWwindow* GetHandle() const noexcept { return m_Handle; }

private:
	void SetupGLState() const;

	GLFWwindow* m_Handle{nullptr};
	int m_Width{1280};
	int m_Height{720};
	int m_WindowedX{100};
	int m_WindowedY{100};
	int m_WindowedWidth{1280};
	int m_WindowedHeight{720};
	bool m_IsFullscreen{false};
	std::string m_Title{"GLEngine"};
};

} // namespace Core