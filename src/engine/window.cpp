#include "engine/window.hpp"
#include "GLFW/glfw3.h"
#include <string>


using namespace Engine;

Engine::Window::Window(uint32_t width, uint32_t height, std::string_view title)
	: ptr() {
	std::scoped_lock windowCreationLock{_windowMtx};
	[[maybe_unused]] static bool glfwIniter = []() {
		glfwInit();
		return true;
	}();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	ptr = glfwCreateWindow(width, height, std::string(title).c_str(), nullptr, nullptr);
}

void Engine::Window::destroy() {
	if (destroyed) return;
	destroyed = true;
	glfwDestroyWindow(ptr);
}

Engine::Window::~Window() {
	if (!destroyed) destroy();
}
