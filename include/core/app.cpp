#include "app.hpp"
#include "fontStore.hpp"
#include <GLFW/glfw3.h>


namespace squi::core {
	void App::run() {
		auto &window = engine.instance.window.ptr;
		{
			std::scoped_lock lock{Engine::Window::_windowMtx};
			windowMap[window] = this;
			glfwSetWindowMaximizeCallback(engine.instance.window.ptr, [](GLFWwindow *windowPtr, int maximized) {
				std::scoped_lock wnd{windowMapMtx};
				auto *window = windowMap[windowPtr];
				if (window->maximizeCallback) window->maximizeCallback(static_cast<bool>(maximized));
			});
			glfwSetFramebufferSizeCallback(engine.instance.window.ptr, [](GLFWwindow *windowPtr, int width, int height) {
				std::scoped_lock wnd{windowMapMtx};
				auto *window = windowMap[windowPtr];
				std::scoped_lock swp{window->engine.swapChainMtx};
				if (window->resizeCallback) window->resizeCallback(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
				window->engine.resized = true;
				window->inputQueue.push(StateChange{});
			});
			glfwSetCursorPosCallback(window, [](GLFWwindow *m_window, double xpos, double ypos) {
				std::scoped_lock _{windowMapMtx};
				auto *window = App::windowMap.at(m_window);
				window->inputQueue.push(CursorPosInput{
					.xPos = static_cast<float>(xpos),
					.yPos = static_cast<float>(ypos),
				});
			});
			glfwSetCharCallback(window, [](GLFWwindow *m_window, unsigned int codepoint) {
				std::scoped_lock _{windowMapMtx};
				auto &window = App::windowMap.at(m_window);
				window->inputQueue.push(CodepointInput{
					.character = static_cast<char>(codepoint),
				});
			});
			glfwSetScrollCallback(window, [](GLFWwindow *m_window, double xoffset, double yoffset) {
				std::scoped_lock _{windowMapMtx};
				auto &window = App::windowMap.at(m_window);
				window->inputQueue.push(ScrollInput{
					.xOffset = static_cast<float>(xoffset),
					.yOffset = static_cast<float>(yoffset),
				});
			});
			glfwSetKeyCallback(window, [](GLFWwindow *m_window, int key, int /*scancode*/, int action, int mods) {
				std::scoped_lock _{windowMapMtx};
				//		Screen::getCurrentScreen()->animationRunning();
				auto &window = App::windowMap.at(m_window);
				window->inputQueue.push(KeyInput{
					.key = key,
					.action = action,
					.mods = mods,
				});
			});
			glfwSetMouseButtonCallback(window, [](GLFWwindow *m_window, int button, int action, int mods) {
				std::scoped_lock _{windowMapMtx};
				//		Screen::getCurrentScreen()->animationRunning();
				auto &window = App::windowMap.at(m_window);
				window->inputQueue.push(MouseInput{
					.button = button,
					.action = action,
					.mods = mods,
				});
			});
			glfwSetCursorEnterCallback(window, [](GLFWwindow *m_window, int entered) {
				std::scoped_lock _{windowMapMtx};
				auto &window = App::windowMap.at(m_window);
				window->inputQueue.push(CursorEntered{
					.entered = static_cast<bool>(entered),
				});
			});
		}
		std::thread([&]() {
			rootElement->mount(nullptr);
			auto &renderObjectElem = dynamic_cast<RenderObjectElement &>(*rootElement);
			auto &renderObject = *renderObjectElem.renderObject;
			engine.run(
				[&]() -> bool {
					static thread_local bool firstRun = true;
					if (!firstRun && inputQueue.waitForInput())
						inputState.parseInput(inputQueue.pop());

					inputState.frameBegin();
					firstRun = false;
					if (engine.resized || engine.outdatedFramebuffer) {
						engine.recreateSwapChain();
						needsRelayout = true;
					}
					drewLastFrame = false;

					const auto &width = engine.instance.swapChainExtent.width;
					const auto &height = engine.instance.swapChainExtent.height;
					// state.width = static_cast<float>(width);
					// state.height = static_cast<float>(height);
					// state.root = this;

					inputState.g_activeArea.emplace_back(
						vec2{0.0f, 0.0f},
						vec2{static_cast<float>(width), static_cast<float>(height)}
					);

					// for (const auto &[_, ptr]: cleanupQueue) {
					// 	auto w = ptr.lock();
					// 	if (w) {
					// 		w->pruneChildren();
					// 	}
					// }
					// cleanupQueue.clear();

					if (needsRelayout) {
						renderObject.calculateSize(
							BoxConstraints{
								.maxWidth = static_cast<float>(width),
								.maxHeight = static_cast<float>(height),
							},
							true
						);
						// std::println("Relayout counter:");
						// for (const auto &[key, value]: relayoutCounter) {
						// 	std::println("{} - {} layouts", key, value);
						// }
						// relayoutCounter.clear();
					}

					if (needsRelayout || needsReposition) {
						renderObject.positionAt({0.0f, 0.0f});
					}

					inputState.g_activeArea.pop_back();
					if (!inputState.g_activeArea.empty()) throw std::runtime_error("Missing active area popback!");

					bool forceRedraw = inputState.isKeyPressedOrRepeat(GLFW_KEY_F9);

					{
						std::scoped_lock lock{Engine::Window::_windowMtx};
						inputState.frameEnd();
					}

					for (const auto &font: FontStore::fonts()) {
						auto fontPtr = font.second.lock();
						if (!fontPtr) continue;

						fontPtr->writePendingTextures();
					}

					if (needsRedraw || needsRelayout || needsReposition || forceRedraw) {
						needsRedraw = false;
						needsRelayout = false;
						needsReposition = false;

						drewLastFrame = true;
						return true;
					}

					return false;
				},
				[&]() {
					renderObject.draw();
				},
				[&]() {
					rootElement->unmount();
					std::println("Cleaning up!");
				}
			);

			std::println("Cleaned up!");
			std::scoped_lock _{windowMapMtx};
			windowsToDestroy.emplace_back(this);
			// Wake up the main thread in order to get it to close the window
			// Otherwise it will just wait until there is another input before doing so
			// leading to a circumstance where you can click the close button but the window stays
			glfwPostEmptyEvent();
		}).detach();

		while (!windowMap.empty()) {
			{
				std::scoped_lock _{windowMapMtx};
				for (auto *window: windowsToDestroy) {
					windowMap.erase(window->engine.instance.window.ptr);
					// glfwHideWindow(engine.instance.window.ptr);
					window->engine.instance.window.destroy();
				}
			}
			glfwWaitEvents();
		}

		for (const auto &[key, font]: FontStore::fonts()) {
			if (!font.expired()) {
				std::println("Found non expired font, {} uses", font.use_count());
			}
		}
	}

	void RootWidget::Element::mount(squi::core::Element *parent) {
		squi::core::SingleChildRenderObjectElement::mount(parent);
	}

	void RootWidget::Element::unmount() {
		squi::core::SingleChildRenderObjectElement::unmount();
	}
}// namespace squi::core