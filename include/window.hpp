#pragma once

#include "inputState.hpp"
#include "observer.hpp"
#include <future>
#include <memory>
#include <stdexcept>

#include "engine/engine.hpp"
#include "store/pipeline.hpp"
#include "store/sampler.hpp"
#include "widget.hpp"


#include "chrono"// IWYU pragma: export
using namespace std::chrono_literals;

namespace squi {
	class Window : public Widget {
	public:
		Engine::Runner engine;
		std::unordered_map<uint64_t, uint64_t> relayoutCounter{};
		Store::Sampler samplerStore{};
		Store::Pipeline pipelineStore{};
		InputState inputState{};

		std::mutex inputMtx{};
		bool inputTriggered = false;
		std::promise<void> inputReady{};

	private:
		bool needsRedraw = true;
		bool needsRelayout = true;
		bool needsReposition = true;
		bool drewLastFrame = false;
		Observable<Child> addedChildren{};
		Observable<Child> addedOverlays{};

		static inline std::atomic<uint32_t> windowCount = 0;

		static void glfwError(int id, const char *description);

		static inline std::mutex windowMapMtx{};
		static inline std::mutex pollMtx{};
		static inline std::unordered_map<GLFWwindow *, Window *> windowMap{};
		static inline std::vector<Window *> windowsToDestroy{};

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

		static void run();

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
