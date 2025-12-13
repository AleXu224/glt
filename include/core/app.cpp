#include "app.hpp"
#include "fontStore.hpp"
#include "widgets/layoutInspector.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>


#ifdef _WIN32
#include "dwmapi.h"
#endif


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
					.key = static_cast<GestureKey>(key),
					.action = static_cast<GestureAction>(action),
					.mods = mods,
				});
			});
			glfwSetMouseButtonCallback(window, [](GLFWwindow *m_window, int button, int action, int mods) {
				std::scoped_lock _{windowMapMtx};
				//		Screen::getCurrentScreen()->animationRunning();
				auto &window = App::windowMap.at(m_window);
				window->inputQueue.push(MouseInput{
					.button = static_cast<GestureMouseKey>(button),
					.action = static_cast<GestureAction>(action),
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

#ifdef _WIN32
		bool supportsNewMica = false;
		const auto *const system = L"kernel32.dll";
		DWORD dummy = 0;
		const auto cbInfo = ::GetFileVersionInfoSizeExW(FILE_VER_GET_NEUTRAL, system, &dummy);
		std::vector<char> buffer(cbInfo);
		::GetFileVersionInfoExW(FILE_VER_GET_NEUTRAL, system, dummy, buffer.size(), buffer.data());
		void *p = nullptr;
		UINT size = 0;
		::VerQueryValueW(buffer.data(), L"\\", &p, &size);
		assert(size >= sizeof(VS_FIXEDFILEINFO));
		assert(p != nullptr);
		const auto *pFixed = static_cast<const VS_FIXEDFILEINFO *>(p);

		bool isWin11 = false;
		if (HIWORD(pFixed->dwFileVersionMS) == 10 && HIWORD(pFixed->dwFileVersionLS) >= 22000) {
			isWin11 = true;
			if (HIWORD(pFixed->dwFileVersionLS) >= 22523) supportsNewMica = true;
		}

		HWND hwnd = glfwGetWin32Window(window);
		// Renderer::getInstance().initialize(hwnd, 800, 600);

		int darkMode = 1;
		int mica = 2;
		int micaOld = 1;
		if (isWin11) {
			DwmSetWindowAttribute(hwnd, 20, &darkMode, sizeof(darkMode));
			if (supportsNewMica)
				DwmSetWindowAttribute(hwnd, 38, &mica, sizeof(mica));
			else
				DwmSetWindowAttribute(hwnd, 1029, &micaOld, sizeof(micaOld));
		}
#endif

		std::thread([&]() {
			rootElement->mount(nullptr, 0, 0);
			auto &renderObjectElem = dynamic_cast<RenderObjectElement &>(*rootElement);
			auto &renderObject = *renderObjectElem.renderObject;
			engine.run(
				[&]() -> bool {
					static thread_local bool firstRun = true;
					if (!runningAnimations.empty() || (!firstRun && inputQueue.waitForInput()))
						inputState.parseInput(inputQueue.pop());

					inputState.frameBegin();
					firstRun = false;
					if (engine.resized || engine.outdatedFramebuffer) {
						engine.recreateSwapChain();
						needsRelayout = true;
					}
					drewLastFrame = false;

					auto newFrameStartTime = std::chrono::steady_clock::now();
					deltaTime = newFrameStartTime - frameStartTime;
					frameStartTime = newFrameStartTime;

					const auto &width = engine.instance.swapChainExtent.width;
					const auto &height = engine.instance.swapChainExtent.height;
					// state.width = static_cast<float>(width);
					// state.height = static_cast<float>(height);
					// state.root = this;

					inputState.g_activeArea.emplace_back(
						vec2{0.0f, 0.0f},
						vec2{static_cast<float>(width), static_cast<float>(height)}
					);

					// Update animations
					for (auto it = runningAnimations.begin(); it != runningAnimations.end();) {
						auto *anim = *it;
						anim->markElementDirty();
						if (anim->isCompleted()) {
							it = runningAnimations.erase(it);
						} else {
							++it;
						}
					}

					renderObject.update();

					while (!dirtyElements.empty() || needsRelayout) {
						while (!dirtyElements.empty()) {
							auto it = dirtyElements.begin();
							auto elem = it->second.lock();
							if (!elem) {
								dirtyElements.erase(it);
								continue;
							}
							dirtyElements.erase(it);
							if (elem->mounted && elem->dirty)
								elem->rebuild();
						}
						dirtyElements.clear();

						if (needsRelayout || needsReposition) needsRedraw = true;
						if (needsRelayout) needsReposition = true;

						// if (needsRelayout) {
						// 	std::println("Relayout needed");
						// } else if (needsReposition) {
						// 	std::println("Reposition needed");
						// } else if (needsRedraw) {
						// 	std::println("Redraw needed");
						// }

						// std::println("Needs relayout: {}, Needs reposition: {}, Needs redraw: {}", needsRelayout, needsReposition, needsRedraw);

						if (needsRelayout) {
							renderObject.calculateSize(
								BoxConstraints{
									.maxWidth = static_cast<float>(width),
									.maxHeight = static_cast<float>(height),
								},
								true
							);
						}
						needsRelayout = false;

						for (const auto &task: postLayoutTasks) {
							task();
						}
						postLayoutTasks.clear();

						if (needsReposition) {
							renderObject.positionAt(
								Rect::fromPosSize(
									vec2{0.0f, 0.0f},
									vec2{static_cast<float>(width), static_cast<float>(height)}
								)
							);
						}

						for (const auto &task: postRepositionTasks) {
							task();
						}
						postRepositionTasks.clear();

						needsReposition = false;
					}

					inputState.g_activeArea.pop_back();
					if (!inputState.g_activeArea.empty()) throw std::runtime_error("Missing active area popback!");

					bool forceRedraw = inputState.isKeyPressedOrRepeat(GestureKey::f9);

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

	Child RootWidget::Element::build() {
		if (auto rootWidget = std::static_pointer_cast<RootWidget>(widget)) {
			return LayoutInspector{
				.child = rootWidget->child,
			};
		}
		return nullptr;
	}

	void RootWidget::Element::mount(squi::core::Element *parent, size_t index, size_t depth) {
		squi::core::SingleChildRenderObjectElement::mount(parent, index, depth);
	}

	void RootWidget::Element::unmount() {
		squi::core::SingleChildRenderObjectElement::unmount();
	}
}// namespace squi::core