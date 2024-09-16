#pragma once

#include "cstdint"
#include "mutex"
#include <string_view>


struct GLFWwindow;

namespace Engine {
	struct Window {
		static inline std::mutex _windowMtx{};

		GLFWwindow *ptr;
		bool destroyed = false;

		Window(const Window &) = default;
		Window(Window &&) = delete;
		Window &operator=(const Window &) = default;
		Window &operator=(Window &&) = delete;

		Window(uint32_t width, uint32_t height, std::string_view title);

		void destroy();

		~Window();
	};
}// namespace Engine