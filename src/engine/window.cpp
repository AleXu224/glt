#include "engine/window.hpp"
#include <string>

using namespace Engine;

Engine::Window::Window(uint32_t width, uint32_t height, std::string_view title)
	: ptr() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	ptr = glfwCreateWindow(width, height, std::string(title).c_str(), nullptr, nullptr);
}