#pragma once

#include "observer.hpp"
#include <memory>
#include <stdexcept>

#include "engine/engine.hpp"
#include "widget.hpp"

#include "chrono"// IWYU pragma: export
using namespace std::chrono_literals;

namespace squi {
	struct MemoizedSize{
		std::optional<vec2> noShrink{};
		std::optional<vec2> verticalShrink{};
		std::optional<vec2> horizontalShrink{};
		std::optional<vec2> allShrink{};

		[[nodiscard]] static auto fromShouldShrink(Widget::ShouldShrink shouldShrink) {
			if (shouldShrink.width && shouldShrink.height) return &MemoizedSize::allShrink;
			if (shouldShrink.width) return &MemoizedSize::horizontalShrink;
			if (shouldShrink.height) return &MemoizedSize::verticalShrink;
			return &MemoizedSize::noShrink;
		}
	};
	class Window : public Widget {
	public:
		Engine::Vulkan engine;
		std::unordered_map<uint64_t, uint64_t> relayoutCounter{};
		std::unordered_map<uint64_t, MemoizedSize> memoisedSizes{};

	private:
		bool needsRedraw = true;
		bool needsRelayout = true;
		bool needsReposition = true;
		bool drewLastFrame = false;
		Observable<Child> addedChildren{};
		Observable<Child> addedOverlays{};


		static void glfwError(int id, const char *description);

		static std::unordered_map<GLFWwindow *, Window *> windowMap;

		Child content{};

	public:
		Window();
		~Window() override;

		static Window &of(Widget *w) {
			if (!*w->state.root) {
				throw std::runtime_error("Widget has invalid root!");
			}
			return *dynamic_cast<Window *>(*w->state.root);
		}
		static Window &of(const std::weak_ptr<Widget> &w) {
			if (w.expired()) throw std::runtime_error("Weak pointer has expired");
			Widget &widgetRef = *w.lock();
			if (!*widgetRef.state.root) {
				throw std::runtime_error("Widget has invalid root!");
			}
			return *dynamic_cast<Window *>(*widgetRef.state.root);
		}

		void run();

		Window(const Window &) = delete;
		Window &operator=(const Window &) = delete;
		Window(Window &&) = delete;
		Window &operator=(Window &&) = delete;

		operator GLFWwindow *() const { return engine.instance.window.ptr; }

		void addOverlay(const Child &overlay) {
			addedOverlays.notify(overlay);
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
