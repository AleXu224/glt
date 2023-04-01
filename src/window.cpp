#include "window.hpp"
#include "batch.hpp"
#include "chrono"
#include "fontStore.hpp"
#include "quad.hpp"
#include "random"
#include "ranges"
#include "stdexcept"
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include "renderer.hpp"

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

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window.reset(glfwCreateWindow(800, 600, "Window", nullptr, nullptr), [](GLFWwindow *windowPtr) {
		glfwDestroyWindow(windowPtr);
		glfwTerminate();
	});

	if (!window) {
		throw std::runtime_error("Failed to create window");
		return;
	}

	glfwSetFramebufferSizeCallback(window.get(), [](GLFWwindow *windowPtr, int width, int height) {
		  // TODO: resize directx context
		auto &renderer = Renderer::getInstance();
		renderer.updateScreenSize(width, height);
//		  Renderer::initialize(glfwGetWin32Window(windowPtr), width, height);
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

	HWND hwnd = glfwGetWin32Window(window.get());
	Renderer::initialize(hwnd, 800, 600);
}

void Window::run() {
	Renderer &renderer = Renderer::getInstance();

	auto lastTime = std::chrono::steady_clock::now();

	unsigned int fps = 0;
	auto lastFpsTime = std::chrono::steady_clock::now();
	while (!glfwWindowShouldClose(window.get())) {
		auto currentFpsTime = std::chrono::steady_clock::now();
		auto fpsDeltaTime = currentFpsTime - lastFpsTime;
		if (fpsDeltaTime > 1s) {
			glfwSetWindowTitle(window.get(), std::to_string(fps).c_str());
			fps = 0;
			lastFpsTime = currentFpsTime;
		}
		fps++;
		auto context = renderer.getDeviceContext();
		glfwPollEvents();

		float color[] = {0.0f, 0.2f, 0.4f, 1.0f};
		auto *renderTargetView = renderer.getRenderTargetView().get();
		context->ClearRenderTargetView(renderTargetView, color);

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

		auto *swapChain = renderer.getSwapChain().get();
		swapChain->Present(0, 0);
		lastTime = currentTime;
	}
}