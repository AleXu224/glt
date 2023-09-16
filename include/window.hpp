#pragma once

#include "chrono"
#include "observer.hpp"
#include <memory>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "widget.hpp"

using namespace std::chrono_literals;

namespace squi {
	class Window : public Widget {
		std::shared_ptr<GLFWwindow> window;
		std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point lastFpsTime = std::chrono::steady_clock::now();
		uint32_t fps = 0;
		bool isWin11 = false;
		bool needsRedraw = true;
		bool needsRelayout = true;
		bool needsReposition = true;
		bool drewLastFrame = false;
		std::shared_ptr<Observable<Child>> addedChildren = Observable<Child>::create();
		std::shared_ptr<Observable<Child>> addedOverlays = Observable<Child>::create();

		static void glfwError(int id, const char *description);

		static std::unordered_map<GLFWwindow *, Window *> windowMap;

		Child content{};

		void updateAndDraw();


	public:
		Window();
		~Window() override;

		void run();

		Window(const Window &) = delete;
		Window &operator=(const Window &) = delete;

		operator GLFWwindow *() const { return window.get(); }

		void addOverlay(Child &&overlay) {
			addedOverlays->notify(overlay);
		}

		void shouldRedraw() {
			needsRedraw = true;
		}

		void shouldRelayout() {
			needsRelayout = true;
		}

		void shouldReposition() {
			needsReposition = true;
		}
	};
}// namespace squi
