#pragma once

#include "observer.hpp"
#include <memory>
#include <stdexcept>

#include "engine/engine.hpp"
#include "widget.hpp"

#include "chrono"// IWYU pragma: export
using namespace std::chrono_literals;

namespace squi {
	class Window : public Widget {
	public:
		Engine::Vulkan engine;

	private:
		bool needsRedraw = true;
		bool needsRelayout = true;
		bool needsReposition = true;
		bool drewLastFrame = false;
		std::shared_ptr<Observable<Child>> addedChildren = Observable<Child>::create();
		std::shared_ptr<Observable<Child>> addedOverlays = Observable<Child>::create();

		static void glfwError(int id, const char *description);

		static std::unordered_map<GLFWwindow *, Window *> windowMap;

		Child content{};

	public:
		Window();
		~Window() override;

		static Window &of(Widget *w) {
			if (!*w->state.root) throw std::runtime_error("Widget has invalid root!");
			return *dynamic_cast<Window *>(*w->state.root);
		}
		static Window &of(const std::weak_ptr<Widget> &w) {
			if (w.expired()) throw std::runtime_error("Weak pointer has expired");
			Widget &widgetRef = *w.lock();
			if (!*widgetRef.state.root) throw std::runtime_error("Widget has invalid root!");
			return *dynamic_cast<Window *>(*widgetRef.state.root);
		}

		void run();

		Window(const Window &) = delete;
		Window &operator=(const Window &) = delete;
		Window(Window &&) = delete;
		Window &operator=(Window &&) = delete;

		operator GLFWwindow *() const { return engine.instance.window.ptr; }

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
