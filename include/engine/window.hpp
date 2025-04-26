#pragma once

#include "cstdint"
#include "mutex"


struct GLFWwindow;

namespace Engine {
	struct WindowOptions {
		std::string name = "Glt Window";
		uint32_t width = 800;
		uint32_t height = 600;
		bool maximized = false;
	};

	struct Window {
		static inline std::mutex _windowMtx{};

		GLFWwindow *ptr;
		bool destroyed = false;

		Window(const Window &) = default;
		Window(Window &&) = delete;
		Window &operator=(const Window &) = default;
		Window &operator=(Window &&) = delete;

		Window(WindowOptions options);

		void destroy();

		~Window();
	};
}// namespace Engine