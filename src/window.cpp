#ifdef NDEBUG
#ifdef _WIN32
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif

#include "window.hpp"
#include "chrono"
#include "gestureDetector.hpp"
#include "layoutInspector.hpp"
#include "widget.hpp"
#include <GLFW/glfw3.h>
#include <print>
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include "dwmapi.h"
#include "renderer.hpp"

using namespace squi;

void Window::glfwError(int id, const char *description) {
	std::print("GLFW Error {}: {}\n", id, description);
}

std::unordered_map<GLFWwindow *, Window *> Window::windowMap{};

Window::Window() : Widget(Widget::Args{}, Widget::Flags{
											  .shouldLayoutChildren = false,
											  .shouldArrangeChildren = false,
											  .isInteractive = false,
										  }) {
	glfwSetErrorCallback(&glfwError);
	if (!glfwInit()) {
		std::println("Failed to initialize GLFW");
		exit(1);
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

	window.reset(glfwCreateWindow(800, 600, "Window", nullptr, nullptr), [](GLFWwindow *windowPtr) {
		glfwDestroyWindow(windowPtr);
		glfwTerminate();
	});

	if (!window) {
		std::println("Failed to create GLFW window");
		exit(1);
	}

	windowMap[window.get()] = this;

	glfwSetFramebufferSizeCallback(window.get(), [](GLFWwindow *windowPtr, int width, int height) {
		auto &renderer = Renderer::getInstance();
		renderer.updateScreenSize(width, height);
		auto *window = windowMap[windowPtr];
		window->shouldRelayout();
		window->updateAndDraw();
	});
	glfwSetCursorPosCallback(window.get(), [](GLFWwindow *m_window, double xpos, double ypos) {
		auto dpiScale = GestureDetector::g_dpi / vec2{96};
		GestureDetector::setCursorPos(vec2{(float) (xpos), (float) (ypos)} / dpiScale);
	});
	glfwSetCharCallback(window.get(), [](GLFWwindow *m_window, unsigned int codepoint) {
		GestureDetector::g_textInput.append(1, static_cast<char>(codepoint));
	});
	glfwSetScrollCallback(window.get(), [](GLFWwindow *m_window, double xoffset, double yoffset) {
		GestureDetector::g_scrollDelta += vec2{static_cast<float>(xoffset), static_cast<float>(yoffset)};
	});
	glfwSetKeyCallback(window.get(), [](GLFWwindow *m_window, int key, int scancode, int action, int mods) {
		//		Screen::getCurrentScreen()->animationRunning();
		if (!GestureDetector::g_keys.contains(key))
			GestureDetector::g_keys.insert({key, {action, mods}});
		else
			GestureDetector::g_keys.at(key) = {action, mods};
	});
	glfwSetMouseButtonCallback(window.get(), [](GLFWwindow *m_window, int button, int action, int mods) {
		//		Screen::getCurrentScreen()->animationRunning();
		if (!GestureDetector::g_keys.contains(button))
			GestureDetector::g_keys.insert({button, {action, mods}});
		else
			GestureDetector::g_keys.at(button) = {action, mods};
	});
	glfwSetCursorEnterCallback(window.get(), [](GLFWwindow *m_window, int entered) {
		GestureDetector::g_cursorInside = static_cast<bool>(entered);
	});

	bool supportsNewMica = false;

	const auto system = L"kernel32.dll";
	DWORD dummy;
	const auto cbInfo =
		::GetFileVersionInfoSizeExW(FILE_VER_GET_NEUTRAL, system, &dummy);
	std::vector<char> buffer(cbInfo);
	::GetFileVersionInfoExW(FILE_VER_GET_NEUTRAL, system, dummy,
							buffer.size(), &buffer[0]);
	void *p = nullptr;
	UINT size = 0;
	::VerQueryValueW(buffer.data(), L"\\", &p, &size);
	assert(size >= sizeof(VS_FIXEDFILEINFO));
	assert(p != nullptr);
	auto pFixed = static_cast<const VS_FIXEDFILEINFO *>(p);

	if (HIWORD(pFixed->dwFileVersionMS) == 10 && HIWORD(pFixed->dwFileVersionLS) >= 22000) {
		isWin11 = true;
		if (HIWORD(pFixed->dwFileVersionLS) >= 22523) supportsNewMica = true;
	}

	HWND hwnd = glfwGetWin32Window(window.get());
	Renderer::getInstance().initialize(hwnd, 800, 600);

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

	content = Child(LayoutInspector{
		.addedChildren = addedChildren,
		.addedOverlays = addedOverlays,
	});
}

Window::~Window() {
	windowMap.erase(window.get());
}

void Window::run() {
	while (!glfwWindowShouldClose(window.get())) {
		updateAndDraw();
	}
}

void Window::updateAndDraw() {
	Renderer &renderer = Renderer::getInstance();
	auto currentFpsTime = std::chrono::steady_clock::now();
	auto fpsDeltaTime = currentFpsTime - lastFpsTime;
	if (fpsDeltaTime > 1s) {
		glfwSetWindowTitle(window.get(), std::to_string(fps).c_str());
		fps = 0;
		lastFpsTime = currentFpsTime;
	}
	fps++;
	auto context = renderer.getDeviceContext();
	const auto beforePoll = std::chrono::steady_clock::now();
	// glfwPollEvents();
	if (drewLastFrame) glfwPollEvents();
	else
		glfwWaitEventsTimeout(1.0 / 10.0);// Run at 10 fps
	drewLastFrame = false;

	const auto afterPoll = std::chrono::steady_clock::now();

	const auto color = [isWin11 = isWin11]() -> std::array<float, 4> {
		if (isWin11) return {0.0f, 0.0f, 0.0f, 0.0f};
		else
			return {32.f / 255.f, 32.f / 255.f, 32.f / 255.f, 1.0f};
	}();
	auto *renderTargetView = renderer.getRenderTargetView().get();
	context->ClearRenderTargetView(renderTargetView, color.data());

	auto &children = getChildren();
	int width, height;
	glfwGetWindowSize(window.get(), &width, &height);
	setWidth(static_cast<float>(width));
	setHeight(static_cast<float>(height));
	state.root = this;

	auto currentTime = std::chrono::steady_clock::now();
	auto deltaTime = currentTime - lastTime;

	renderer.updateDeltaTime(deltaTime);
	renderer.updateCurrentFrameTime(currentTime);

	for (auto &child: children) {
		addedChildren->notify(child);
	}
	children.clear();

	uint32_t hitChecks = 0;

	GestureDetector::g_activeArea.emplace_back(
		vec2{0.0f, 0.0f},
		vec2{static_cast<float>(width), static_cast<float>(height)});


	content->state.parent = this;
	content->state.root = this;
	content->update();

	if (needsRelayout)
		content->layout({static_cast<float>(width), static_cast<float>(height)}, {});

	if (needsRelayout || needsReposition)
		content->arrange({0.0f, 0.0f});

	for (uint32_t i = 0; i < hitChecks; i++) {
		GestureDetector::g_hitCheckRects.pop_back();
	}

	GestureDetector::g_activeArea.pop_back();
	if (!GestureDetector::g_activeArea.empty()) printf("active area not empty\n");

	const auto afterUpdateTime = std::chrono::steady_clock::now();

	renderer.prepare();

	if (needsRedraw || needsRelayout || needsReposition) {
		content->draw();
		renderer.render();
	}

	renderer.popClipRect();
	const auto afterDrawTime = std::chrono::steady_clock::now();

	auto *swapChain = renderer.getSwapChain().get();
	if (needsRedraw || needsRelayout || needsReposition) {
		swapChain->Present(1, 0);
		drewLastFrame = true;
	}
	lastTime = currentTime;

	static uint32_t relayoutCount = 0;
	static uint32_t repositionCount = 0;
	static uint32_t redrawCount = 0;

	if (needsRelayout)
		std::print("needsRelayout ({})\n", relayoutCount++);
	else if (needsReposition)
		std::print("needsReposition ({})\n", repositionCount++);
	else if (needsRedraw)
		std::print("needsRedraw ({})\n", redrawCount++);

	needsRedraw = false;
	needsRelayout = false;
	needsReposition = false;

	const auto afterPresentTime = std::chrono::steady_clock::now();

	renderer.updatePollTime(afterPoll - beforePoll);
	renderer.updateUpdateTime(afterUpdateTime - currentTime);
	renderer.updateDrawTime(afterDrawTime - afterUpdateTime);
	renderer.updatePresentTime(afterPresentTime - afterDrawTime);

	GestureDetector::frameEnd();
}