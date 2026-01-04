#include "engine/window.hpp"
#include "GLFW/glfw3.h"
#include <string>


using namespace Engine;

Engine::Window::Window(WindowOptions options)
	: ptr() {
	std::scoped_lock windowCreationLock{_windowMtx};
	[[maybe_unused]] static bool glfwIniter = []() {
		glfwInit();
		return true;
	}();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_MAXIMIZED, options.maximized);
	// glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	ptr = glfwCreateWindow(options.width, options.height, options.name.c_str(), nullptr, nullptr);
}

void Engine::Window::destroy() {
	if (destroyed) return;
	destroyed = true;
	glfwDestroyWindow(ptr);
	destroyPromise.set_value();
}

Engine::Window::~Window() {
	if (!destroyed) destroy();
}
