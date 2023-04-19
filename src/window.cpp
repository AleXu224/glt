// #ifdef NDEBUG
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
// #endif

#include "window.hpp"
#include "chrono"
#include "fontStore.hpp"
#include "gestureDetector.hpp"
#include "overlay.hpp"
#include "ranges"
#include "stdexcept"
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include "VersionHelpers.h"
#include "dwmapi.h"
#include "renderer.hpp"

using namespace squi;

void Window::glfwError(int id, const char *description) {
	printf("GLFW Error %d: %s\n", id, description);
}

std::unordered_map<GLFWwindow *, Window *> Window::windowMap{};

Window::Window() : Widget(Widget::Args{}, Widget::Options{
	.shouldHandleLayout = false,
	.shouldArrangeChildren = false,
	.isInteractive = false,
	}) {
	glfwSetErrorCallback(&glfwError);
	if (!glfwInit()) {
		printf("Failed to initialize GLFW\n");
		exit(1);
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

	window.reset(glfwCreateWindow(800, 600, "Window", nullptr, nullptr), [](GLFWwindow *windowPtr) {
		glfwDestroyWindow(windowPtr);
		glfwTerminate();
	});

	if (!window) {
		printf("Failed to create GLFW window\n");
		exit(1);
	}

	windowMap[window.get()] = this;

	glfwSetFramebufferSizeCallback(window.get(), [](GLFWwindow *windowPtr, int width, int height) {
		auto &renderer = Renderer::getInstance();
		renderer.updateScreenSize(width, height);
		auto *window = windowMap[windowPtr];
		window->updateAndDraw();
	});
	glfwSetCursorPosCallback(window.get(), [](GLFWwindow *m_window, double xpos, double ypos) {
		auto dpiScale = GestureDetector::g_dpi / vec2{96};
		GestureDetector::setCursorPos(vec2{(float) (xpos), (float) (ypos)} / dpiScale);
	});
	glfwSetCharCallback(window.get(), [](GLFWwindow *m_window, unsigned int codepoint) {
		GestureDetector::g_textInput = static_cast<unsigned char>(codepoint);
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
	Renderer::initialize(hwnd, 800, 600);

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
}

Window::~Window() {
	Widget::~Widget();
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
	glfwPollEvents();
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
	auto &widgetData = data();
	widgetData.sizeMode = {static_cast<float>(width), static_cast<float>(height)};

	auto currentTime = std::chrono::steady_clock::now();
	auto deltaTime = currentTime - lastTime;
	constexpr bool UPDATE_DEBUG_STEP = false;
	if (!UPDATE_DEBUG_STEP || GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_W)) {
		renderer.updateDeltaTime(deltaTime);
		renderer.updateCurrentFrameTime(currentTime);
		uint32_t hitChecks = 0;
		layout({static_cast<float>(width), static_cast<float>(height)});
		arrange({0.0f, 0.0f});
		for (auto &overlay: std::views::reverse(overlays)) {
			overlay->data().parent = this;
			overlay->update();
			overlay->layout({static_cast<float>(width), static_cast<float>(height)});
			overlay->arrange({0.0f, 0.0f});
			auto hitCheck = overlay->getHitcheckRect();
			if (hitCheck.has_value()) {
				GestureDetector::g_hitCheckRects.emplace_back(hitCheck.value());
				hitChecks++;
			}
		}

		for (auto &child: std::views::reverse(children)) {
			child->data().parent = this;
			child->update();
			child->layout({static_cast<float>(width), static_cast<float>(height)});
			child->arrange({0.0f, 0.0f});
			auto hitCheck = child->getHitcheckRect();
			if (hitCheck.has_value()) {
				GestureDetector::g_hitCheckRects.emplace_back(hitCheck.value());
				hitChecks++;
			}
		}

		for (uint32_t i = 0; i < hitChecks; i++) {
			GestureDetector::g_hitCheckRects.pop_back();
		}
	}
	const auto afterUpdateTime = std::chrono::steady_clock::now();

	renderer.prepare();

	for (auto &child: children) {
		child->draw();
	}

	for (auto &overlay: overlays) {
		overlay->draw();
	}

	renderer.render();
	renderer.popClipRect();
	const auto afterDrawTime = std::chrono::steady_clock::now();

	auto *swapChain = renderer.getSwapChain().get();
	swapChain->Present(1, 0);
	lastTime = currentTime;

	const auto afterPresentTime = std::chrono::steady_clock::now();

	renderer.updatePollTime(afterPoll - beforePoll);
	renderer.updateUpdateTime(afterUpdateTime - currentTime);
	renderer.updateDrawTime(afterDrawTime - afterUpdateTime);
	renderer.updatePresentTime(afterPresentTime - afterDrawTime);

	if (!UPDATE_DEBUG_STEP || GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_W)) {
		GestureDetector::g_keys.clear();
		GestureDetector::g_textInput = 0;
		GestureDetector::g_scrollDelta = 0;
		GestureDetector::setCursorPos(GestureDetector::g_cursorPos);
	}
}