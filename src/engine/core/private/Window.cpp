#include "../../utils/public/Logger.h"
#include <engine/core/public/Window.h>

#include "engine/core/public/Game.h"

#include <iostream>

namespace Engine::Core {

Window::~Window() {
	OnWindowResized.Clear();

	if (m_Handle) {
		glfwDestroyWindow(m_Handle);
	}

	glfwTerminate();
}

bool Window::Initialize(const int width, const int height, const std::string_view title) {
	m_Width = width;
	m_Height = height;
	m_Title = title;

	if (!glfwInit()) {
		Utils::Log::Error("Failed to initialize GLFW");
		return false;
	}

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	m_Handle = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
	if (!m_Handle) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		return false;
	}

	glfwMakeContextCurrent(m_Handle);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	SetupGLState();

	glfwSetWindowUserPointer(m_Handle, this);

	glfwSetFramebufferSizeCallback(m_Handle, [](GLFWwindow* window, const int newWidth, const int newHeight) {
		if (newWidth > 0 && newHeight > 0) {
			auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
			self->m_Width = newWidth;
			self->m_Height = newHeight;

			glViewport(0, 0, newWidth, newHeight);

			self->OnWindowResized.Execute();
		}
	});

	glfwSetKeyCallback(m_Handle, [](GLFWwindow*, const int key, int, const int action, int) {
		if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
			Get().ToggleFullscreen();
		}
	});

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback([](GLenum, GLenum, GLuint, const GLenum severity, GLsizei, const GLchar* message, const void*) {
		if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
			CORE_WARN("OpenGL Debug: ", message);
		}
	}, nullptr);

	SetVSync(true);
	glfwShowWindow(m_Handle);
	return true;
}

void Window::ToggleFullscreen() {
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	if (!m_IsFullscreen) {
		glfwGetWindowPos(m_Handle, &m_WindowedX, &m_WindowedY);
		m_WindowedWidth = m_Width;
		m_WindowedHeight = m_Height;

		glfwSetWindowMonitor(m_Handle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		m_IsFullscreen = true;
	}
	else {
		glfwSetWindowMonitor(m_Handle, nullptr, m_WindowedX, m_WindowedY, m_WindowedWidth, m_WindowedHeight, 0);
		m_IsFullscreen = false;
	}
}

bool Window::IsOpen() const {
	return !glfwWindowShouldClose(m_Handle);
}

void Window::SetVSync(const bool enabled) {
	glfwSwapInterval(enabled ? 1 : 0);
}

void Window::SwapBuffers() const {
	glfwSwapBuffers(m_Handle);
}

void Window::PollEvents() {
	glfwPollEvents();
}

float Window::GetAspectRatio() const {
	return static_cast<float>(m_Width) / static_cast<float>(m_Height);
}

void Window::SetupGLState() const {
	glViewport(0, 0, m_Width, m_Height);
}

} // namespace Core