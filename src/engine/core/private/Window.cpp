#include "../../utils/public/Logger.h"
#include <engine/core/public/Window.h>

#include "engine/asset-management/public/AssetManager.h"
#include "stb/stb_image.h"

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

	SetIcons();

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

void Window::SetIcons() const {
	const std::array<std::string, 4>& iconPaths = {
		AssetManagement::EngineAssets::Resolve("textures/icons/tex_icon_16.png"),
		AssetManagement::EngineAssets::Resolve("textures/icons/tex_icon_32.png"),
		AssetManagement::EngineAssets::Resolve("textures/icons/tex_icon_48.png"),
		AssetManagement::EngineAssets::Resolve("textures/icons/tex_icon_256.png"),
	};

	std::vector<GLFWimage> images;
	std::vector<unsigned char*> rawData;

	for (const auto& path : iconPaths) {
		int width, height, channels;

		if (unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4)) {
			GLFWimage image;
			image.width = width;
			image.height = height;
			image.pixels = pixels;

			images.push_back(image);
			rawData.push_back(pixels);
		}
		else {
			Utils::Log::Warn("Can't load icon: ", path);
		}
	}

	if (!images.empty()) {
		glfwSetWindowIcon(m_Handle, static_cast<int>(images.size()), images.data());
	}

	for (auto* pixels : rawData) {
		stbi_image_free(pixels);
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