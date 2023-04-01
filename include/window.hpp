#ifndef SQUI_WINDOW_HPP
#define SQUI_WINDOW_HPP
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "chrono"
#include "memory"
#include "renderer.hpp"
#include "widget.hpp"

using namespace std::chrono_literals;

namespace squi {
	class Window : public Widget{
		std::shared_ptr<GLFWwindow> window;

		static void glfwError(int id, const char *description);

	public:
		Window();

		void run();

		Window(const Window &) = delete;
		Window &operator=(const Window &) = delete;

		operator GLFWwindow *() const { return window.get(); }
	};
}// namespace squi
#endif
