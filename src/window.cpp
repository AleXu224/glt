// #ifdef NDEBUG
// #ifdef _WIN32
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
// #endif
// #endif

#include "window.hpp"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "fontStore.hpp"
#include "layoutInspector.hpp"
#include "widget.hpp"
#include <print>
#include <stdexcept>


#ifdef _WIN32
#include "dwmapi.h"
#endif


using namespace squi;

void squi::Window::glfwError(int id, const char *description) {
	std::print("GLFW Error {}: {}\n", id, description);
}

squi::Window::Window()
	: Widget(
		  Widget::Args{},
		  Widget::FlagsArgs{
			  .shouldLayoutChildren = false,
			  .shouldArrangeChildren = false,
			  .isInteractive = false,
		  }
	  ),
	  engine() {
	windowCount++;
	auto &window = engine.instance.window.ptr;
	state.root = this;
	{
		std::scoped_lock lock{Engine::Window::_windowMtx};
		windowMap[window] = this;
		glfwSetFramebufferSizeCallback(engine.instance.window.ptr, [](GLFWwindow *windowPtr, int /*width*/, int /*height*/) {
			std::scoped_lock wnd{windowMapMtx};
			auto *window = windowMap[windowPtr];
			std::scoped_lock swp{window->engine.swapChainMtx};
			window->engine.resized = true;
			window->inputQueue.push(StateChange{});
		});
		glfwSetCursorPosCallback(window, [](GLFWwindow *m_window, double xpos, double ypos) {
			std::scoped_lock _{windowMapMtx};
			auto *window = Window::windowMap.at(m_window);
			window->inputQueue.push(CursorPosInput{
				.xPos = static_cast<float>(xpos),
				.yPos = static_cast<float>(ypos),
			});
		});
		glfwSetCharCallback(window, [](GLFWwindow *m_window, unsigned int codepoint) {
			std::scoped_lock _{windowMapMtx};
			auto &window = Window::windowMap.at(m_window);
			window->inputQueue.push(CodepointInput{
				.character = static_cast<char>(codepoint),
			});
		});
		glfwSetScrollCallback(window, [](GLFWwindow *m_window, double xoffset, double yoffset) {
			std::scoped_lock _{windowMapMtx};
			auto &window = Window::windowMap.at(m_window);
			window->inputQueue.push(ScrollInput{
				.xOffset = static_cast<float>(xoffset),
				.yOffset = static_cast<float>(yoffset),
			});
		});
		glfwSetKeyCallback(window, [](GLFWwindow *m_window, int key, int /*scancode*/, int action, int mods) {
			std::scoped_lock _{windowMapMtx};
			//		Screen::getCurrentScreen()->animationRunning();
			auto &window = Window::windowMap.at(m_window);
			window->inputQueue.push(KeyInput{
				.key = key,
				.action = action,
				.mods = mods,
			});
		});
		glfwSetMouseButtonCallback(window, [](GLFWwindow *m_window, int button, int action, int mods) {
			std::scoped_lock _{windowMapMtx};
			//		Screen::getCurrentScreen()->animationRunning();
			auto &window = Window::windowMap.at(m_window);
			window->inputQueue.push(MouseInput{
				.button = button,
				.action = action,
				.mods = mods,
			});
		});
		glfwSetCursorEnterCallback(window, [](GLFWwindow *m_window, int entered) {
			std::scoped_lock _{windowMapMtx};
			auto &window = Window::windowMap.at(m_window);
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

	funcs().onChildAdded.emplace_back([](Widget &w, const Child &child) {
		w.as<Window>().addedChildren.notify(child);
	});

	content = Child(LayoutInspector{
		.addedChildren = addedChildren,
		.addedOverlays = addedOverlays,
	});
	content->state.parent = this;
	content->state.root = this;
	std::thread([&]() {
		engine.run(
			[&]() -> bool {
				static thread_local bool firstRun = true;
				if (!firstRun && inputQueue.waitForInput())
					inputState.parseInput(inputQueue.pop());
				firstRun = false;
				if (engine.resized || engine.outdatedFramebuffer) {
					engine.recreateSwapChain();
					needsRelayout = true;
				}
				drewLastFrame = false;

				const auto &width = engine.instance.swapChainExtent.width;
				const auto &height = engine.instance.swapChainExtent.height;
				state.width = static_cast<float>(width);
				state.height = static_cast<float>(height);
				state.root = this;

				getChildren().clear();
				childrenToAdd.clear();

				inputState.g_activeArea.emplace_back(
					vec2{0.0f, 0.0f},
					vec2{static_cast<float>(width), static_cast<float>(height)}
				);


				content->state.parent = this;
				content->state.root = this;
				content->update();
				for (const auto &[_, ptr]: cleanupQueue) {
					auto w = ptr.lock();
					if (w) {
						w->pruneChildren();
					}
				}
				cleanupQueue.clear();

				if (needsRelayout) {
					content->layout({static_cast<float>(width), static_cast<float>(height)}, {}, {}, true);
					// std::println("Relayout counter:");
					// for (const auto &[key, value]: relayoutCounter) {
					// 	std::println("{} - {} layouts", key, value);
					// }
					relayoutCounter.clear();
				}

				if (needsRelayout || needsReposition) {
					content->arrange({0.0f, 0.0f});
				}

				inputState.g_activeArea.pop_back();
				if (!inputState.g_activeArea.empty()) throw std::runtime_error("Missing active area popback!");

				bool forceRedraw = inputState.isKeyPressedOrRepeat(GLFW_KEY_F9);

				{
					std::scoped_lock lock{Engine::Window::_windowMtx};
					inputState.frameEnd();
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
				content->draw();
			},
			[&]() {
				std::println("Cleaning up!");
				this->getPendingChildren().clear();
				this->getChildren().clear();
				this->content.reset();
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
}

squi::Window::~Window() {
	windowCount--;
}

void squi::Window::run() {
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

	if (Widget::getCount() != windowCount) {
		std::println("Widgets still alive: {} (should be 1)", Widget::getCount());
	}
	// FontStore::defaultFont.reset();
	// FontStore::defaultFontBold.reset();
	// FontStore::defaultFontItalic.reset();
	// FontStore::defaultFontBoldItalic.reset();
	for (const auto &[key, font]: FontStore::fonts()) {
		if (!font.expired()) {
			std::println("Found non expired font, {} uses", font.use_count());
		}
	}
}
void squi::Window::queueCleanup(Widget *ptr, ChildRef w) {
	cleanupQueue.emplace(ptr, w);
}
