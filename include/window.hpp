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
		std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point lastFpsTime = std::chrono::steady_clock::now();
		uint32_t fps = 0;
		bool isWin11 = false;

		static void glfwError(int id, const char *description);

		static std::unordered_map<GLFWwindow *, Window *> windowMap;

		void updateAndDraw();

	public:
		Window();
		~Window() override;

		void run();

		Window(const Window &) = delete;
		Window &operator=(const Window &) = delete;

		operator GLFWwindow *() const { return window.get(); }
	};
}// namespace squi
#endif
