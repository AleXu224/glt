// #ifdef NDEBUG
// #ifdef _WIN32
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
// #endif
// #endif

#include "window.hpp"
#include "fontStore.hpp"
#include "gestureDetector.hpp"
#include "layoutInspector.hpp"
#include "widget.hpp"
#include <print>
#include <stdexcept>

#ifdef _WIN32
#include "dwmapi.h"
#endif


using namespace squi;

void Window::glfwError(int id, const char *description) {
	std::print("GLFW Error {}: {}\n", id, description);
}

std::unordered_map<GLFWwindow *, Window *> Window::windowMap{};

Window::Window() : Widget(Widget::Args{}, Widget::FlagsArgs{
											  .shouldLayoutChildren = false,
											  .shouldArrangeChildren = false,
											  .isInteractive = false,
										  }),
				   engine() {
	auto &window = engine.instance.window.ptr;
	state.root = this;

	windowMap[window] = this;

	glfwSetFramebufferSizeCallback(engine.instance.window.ptr, [](GLFWwindow *windowPtr, int /*width*/, int /*height*/) {
		auto *window = windowMap[windowPtr];
		window->engine.resized = true;
		window->engine.draw();
	});
	glfwSetCursorPosCallback(window, [](GLFWwindow * /*m_window*/, double xpos, double ypos) {
		auto dpiScale = GestureDetector::g_dpi / vec2{96};
		GestureDetector::setCursorPos(vec2{(float) (xpos), (float) (ypos)} / dpiScale);
	});
	glfwSetCharCallback(window, [](GLFWwindow * /*m_window*/, unsigned int codepoint) {
		GestureDetector::g_textInput.append(1, static_cast<char>(codepoint));
	});
	glfwSetScrollCallback(window, [](GLFWwindow * /*m_window*/, double xoffset, double yoffset) {
		GestureDetector::g_scrollDelta += vec2{static_cast<float>(xoffset), static_cast<float>(yoffset)};
	});
	glfwSetKeyCallback(window, [](GLFWwindow * /*m_window*/, int key, int /*scancode*/, int action, int mods) {
		//		Screen::getCurrentScreen()->animationRunning();
		if (!GestureDetector::g_keys.contains(key))
			GestureDetector::g_keys.insert({key, {action, mods}});
		else
			GestureDetector::g_keys.at(key) = {action, mods};
	});
	glfwSetMouseButtonCallback(window, [](GLFWwindow * /*m_window*/, int button, int action, int mods) {
		//		Screen::getCurrentScreen()->animationRunning();
		if (!GestureDetector::g_keys.contains(button))
			GestureDetector::g_keys.insert({button, {action, mods}});
		else
			GestureDetector::g_keys.at(button) = {action, mods};
	});
	glfwSetCursorEnterCallback(window, [](GLFWwindow * /*m_window*/, int entered) {
		GestureDetector::g_cursorInside = static_cast<bool>(entered);
	});

	FontStore::initializeDefaultFont(engine.instance);

#ifdef _WIN32
	bool supportsNewMica = false;
	const auto *const system = L"kernel32.dll";
	DWORD dummy = 0;
	const auto cbInfo =
		::GetFileVersionInfoSizeExW(FILE_VER_GET_NEUTRAL, system, &dummy);
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

	content = Child(LayoutInspector{
		.addedChildren = addedChildren,
		.addedOverlays = addedOverlays,
	});
}

Window::~Window() {
	windowMap.erase(engine.instance.window.ptr);
}

void Window::run() {
	content->state.parent = this;
	content->state.root = this;
	engine.run(
		[&]() -> bool {
			if (drewLastFrame) {
				glfwPollEvents();
			} else {
				glfwWaitEventsTimeout(1.0 / 10.0);// Run at 10 fps
			}
			drewLastFrame = false;

			auto &children = getChildren();
			if (engine.resized) {
				needsRelayout = true;
				engine.resized = false;
				// FIXME: Might want to look an alternative way of resizing
				// Recreating the swap chain is way too costly and will result in a laggy resize
				engine.recreateSwapChain();
			}
			const auto &width = engine.instance.swapChainExtent.width;
			const auto &height = engine.instance.swapChainExtent.height;
			state.width = static_cast<float>(width);
			state.height = static_cast<float>(height);
			state.root = this;

			for (auto &child: children) {
				addedChildren->notify(child);
			}
			children.clear();

			GestureDetector::g_activeArea.emplace_back(
				vec2{0.0f, 0.0f},
				vec2{static_cast<float>(width), static_cast<float>(height)}
			);


			content->state.parent = this;
			content->state.root = this;
			content->update();

			if (needsRelayout) {
				content->layout({static_cast<float>(width), static_cast<float>(height)}, {});
			}

			if (needsRelayout || needsReposition) {
				content->arrange({0.0f, 0.0f});
			}

			GestureDetector::g_activeArea.pop_back();
			if (!GestureDetector::g_activeArea.empty()) throw std::runtime_error("Missing active area popback!");

			bool forceRedraw = GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_F9);

			GestureDetector::frameEnd();

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
		[]() {
			FontStore::defaultFont.reset();
		}
	);
}