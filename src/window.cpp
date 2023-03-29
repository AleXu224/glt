#include "window.hpp"
#include "batch.hpp"
#include "chrono"
#include "fontStore.hpp"
#include "quad.hpp"
#include "random"
#include "ranges"
#include "stdexcept"

using namespace squi;

void Window::glfwError(int id, const char *description) {
	printf("GLFW Error %d: %s\n", id, description);
}

Window::Window() : Widget(Widget::Args{}, Widget::Options{.isContainer = false, .isInteractive = false}) {
	glfwSetErrorCallback(&glfwError);
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW");
		return;
	}

	window.reset(glfwCreateWindow(800, 600, "Window", nullptr, nullptr), [](GLFWwindow *windowPtr) {
		glfwDestroyWindow(windowPtr);
		glfwTerminate();
	});

	if (!window) {
		throw std::runtime_error("Failed to create window");
		return;
	}


	glfwSetFramebufferSizeCallback(window.get(), [](GLFWwindow *windowPtr, int width, int height) {
		glViewport(0, 0, width, height);
		auto &renderer = Renderer::getInstance();
		renderer.updateScreenSize(width, height);
		// TODO: Add support for redrawing the window while resizing

		//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//		renderer.render();
		//		glfwSwapBuffers(windowPtr);
	});
	glfwSetCursorPosCallback(window.get(), [](GLFWwindow *m_window, double xpos, double ypos) {
		auto dpiScale = GestureDetector::g_dpi / vec2{96};
		GestureDetector::g_cursorPos = vec2{(float) (xpos), (float) (ypos)} / dpiScale;
	});
	glfwSetCharCallback(window.get(), [](GLFWwindow *m_window, unsigned int codepoint) {
		GestureDetector::g_textInput = static_cast<unsigned char>(codepoint);
	});
	glfwSetScrollCallback(window.get(), [](GLFWwindow *m_window, double xoffset, double yoffset) {
		GestureDetector::g_scrollDelta = vec2{static_cast<float>(xoffset), static_cast<float>(yoffset)};
	});
	glfwSetKeyCallback(window.get(), [](GLFWwindow *m_window, int key, int scancode, int action, int mods) {
		//		Screen::getCurrentScreen()->animationRunning();
		if (!GestureDetector::g_keys.contains(key))
			GestureDetector::g_keys.insert({key, {action, mods}});
		else
			GestureDetector::g_keys.at(key) = {action, mods};
	});
	glfwSetMouseButtonCallback(window.get(), [](GLFWwindow *m_window, int button, int action, int mods) {
		//		Screen::getCurrentScreen()->animationRunning();
		if (!GestureDetector::g_keys.contains(button))
			GestureDetector::g_keys.insert({button, {action, mods}});
		else
			GestureDetector::g_keys.at(button) = {action, mods};
	});
	glfwSetCursorEnterCallback(window.get(), [](GLFWwindow *m_window, int entered) {
		GestureDetector::g_cursorInside = static_cast<bool>(entered);
	});

	glfwMakeContextCurrent(window.get());
	gladLoadGL();
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
		printf("OpenGL Error: %s\n", message);
		printf("Source: %d, Type: %d, Severity: %d\n", source, type, severity);
		exit(1);
	},
						   nullptr);
	glfwSwapInterval(1);
}

void Window::run() {
	Renderer &renderer = Renderer::getInstance();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	auto lastTime = std::chrono::steady_clock::now();
	while (!glfwWindowShouldClose(window.get())) {
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);

		auto &children = getChildren();

		auto currentTime = std::chrono::steady_clock::now();
		auto deltaTime = currentTime - lastTime;
		constexpr bool UPDATE_DEBUG_STEP = false;
		if (!UPDATE_DEBUG_STEP || GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_W)) {
			renderer.updateDeltaTime(deltaTime);
			renderer.updateCurrentFrameTime(currentTime);
			for (auto &child: std::views::reverse(children)) {
				child->data().pos = {0, 0};
				child->data().parent = this;
				child->update();
			}
		}

		for (auto &child: children) {
			child->draw();
		}

		renderer.render();

		glfwSwapBuffers(window.get());
		glFlush();
		lastTime = currentTime;
	}
}