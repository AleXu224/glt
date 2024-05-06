#pragma once

#include <string_view>
#include "cstdint"

struct GLFWwindow;

namespace Engine {
	struct Window {
		GLFWwindow *ptr;

        Window(uint32_t width, uint32_t height, std::string_view title);
	};
}// namespace Engine