#pragma once

#include <string_view>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Engine {
	struct Window {
		GLFWwindow *ptr;

        Window(uint32_t width, uint32_t height, std::string_view title);
	};
}// namespace Engine