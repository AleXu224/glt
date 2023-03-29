#include "gestureDetector.hpp"
#include "widget.hpp"
#include "GLFW/glfw3.h"

using namespace squi;

vec2 GestureDetector::lastCursorPos{0};

vec2 GestureDetector::g_cursorPos{0};
std::unordered_map<int, KeyState> GestureDetector::g_keys{};
unsigned char GestureDetector::g_textInput{0};
vec2 GestureDetector::g_scrollDelta{0};
std::vector<Rect> GestureDetector::g_hitCheckRects{};
vec2 GestureDetector::g_dpi{96};
bool GestureDetector::g_cursorInside{false};

bool GestureDetector::isKey(int key, int action, int mods) {
	if (!g_keys.contains(key)) return false;

	auto &keyInput = g_keys.at(key);
	return (keyInput.action == action && keyInput.mods == mods);
}

bool GestureDetector::isKeyPressedOrRepeat(int key, int mods) {
	if (!g_keys.contains(key)) return false;

	auto &keyInput = g_keys.at(key);
	return ((keyInput.action == GLFW_PRESS || keyInput.action == GLFW_REPEAT) && keyInput.mods == mods);
}

void GestureDetector::update() {
	bool cursorInsideAnotherWidget = false;
	const bool cursorInsideWidget = parent->getRect().contains(g_cursorPos);
	if (cursorInsideWidget) {
		for (auto &widgetRect : g_hitCheckRects) {
			if (widgetRect.contains(g_cursorPos)) {
				cursorInsideAnotherWidget = true;
				break;
			}
		}
	}

	if (g_cursorInside && !cursorInsideAnotherWidget && cursorInsideWidget) {
		scrollDelta = g_scrollDelta;

		if (!hovered && onEnter) onEnter(*this);
		hovered = true;

		if (isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS) && !focusedOutside) {
			if (!focused) dragStart = g_cursorPos;
			focused = true;
			active = true;
		} else if (isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE)) {
			if (focused && !focusedOutside && onClick) onClick(*this);
			focused = false;
			focusedOutside = false;
		}
	} else {
		scrollDelta = vec2{0};

		if (hovered && onLeave) onLeave(*this);
		hovered = false;

		if (isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS) && !focused) {
			focusedOutside = true;
			active = false;
		} else if (isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE)) {
			focused = false;
			focusedOutside = false;
		}
	}

	if (active) charInput = g_textInput;
	else charInput = 0;
	lastCursorPos = g_cursorPos;
}

const vec2 &GestureDetector::getMousePos() {
	return g_cursorPos;
}

vec2 GestureDetector::getMouseDelta() {
	return g_cursorPos - lastCursorPos;
}

const vec2 &GestureDetector::getScroll() const {
	return scrollDelta;
}

vec2 GestureDetector::getDragOffset() const {
	if (!focused) return vec2{0};
	return dragStart - g_cursorPos;
}

const vec2 &GestureDetector::getDragStartPos() const {
	return dragStart;
}