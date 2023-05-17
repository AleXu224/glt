#include "gestureDetector.hpp"
#include "widget.hpp"
#include "GLFW/glfw3.h"
#include <any>

using namespace squi;

vec2 GestureDetector::lastCursorPos{};
vec2 GestureDetector::mouseDelta{0};

vec2 GestureDetector::g_cursorPos{0};
std::unordered_map<int, KeyState> GestureDetector::g_keys{};
unsigned char GestureDetector::g_textInput{0};
vec2 GestureDetector::g_scrollDelta{0};
std::vector<Rect> GestureDetector::g_hitCheckRects{};
std::vector<Rect> GestureDetector::g_activeArea{};
vec2 GestureDetector::g_dpi{96};
bool GestureDetector::g_cursorInside{false};

void GestureDetector::setCursorPos(const vec2 &pos) {
	lastCursorPos = g_cursorPos;
	g_cursorPos = pos;
	mouseDelta = g_cursorPos - lastCursorPos;
}

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

void GestureDetector::Storage::update(Widget &widget) {
	bool cursorInsideAnotherWidget = false;
	const bool cursorInsideWidget = widget.getRect().contains(g_cursorPos);
	const bool cursorInsideActiveArea = g_activeArea.back().contains(g_cursorPos);
	if (cursorInsideWidget && cursorInsideActiveArea) {
		for (auto &widgetRect : g_hitCheckRects) {
			if (widgetRect.contains(g_cursorPos)) {
				cursorInsideAnotherWidget = true;
				break;
			}
		}
	}

	if (g_cursorInside && !cursorInsideAnotherWidget && cursorInsideWidget && cursorInsideActiveArea) {
		scrollDelta = g_scrollDelta;

		if (!hovered && onEnter) onEnter(widget, *this);
		hovered = true;

		if (isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS) && !focusedOutside) {
			if (!focused) {
				dragStart = g_cursorPos;
				if (onPress) onPress(widget, *this);
			}
			focused = true;
			active = true;
		} else if (isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE)) {
			if (focused && !focusedOutside) {
				if (onClick) onClick(widget, *this);
				if (onRelease) onRelease(widget, *this);
			}
			focused = false;
			focusedOutside = false;
		}
	} else {
		scrollDelta = vec2{0};

		if (hovered && onLeave) onLeave(widget, *this);
		hovered = false;

		if (isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS) && !focused) {
			focusedOutside = true;
			active = false;
		} else if (isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE)) {
			focused = false;
			focusedOutside = false;
		}
	}

	if (active && onDrag) onDrag(widget, *this);
	if (active) charInput = g_textInput;
	else charInput = 0;
}

const vec2 &GestureDetector::getMousePos() {
	return g_cursorPos;
}

vec2 GestureDetector::getMouseDelta() {
	return g_cursorPos - lastCursorPos;
}

const vec2 &GestureDetector::Storage::getScroll() const {
	return scrollDelta;
}

vec2 GestureDetector::Storage::getDragDelta() const {
	if (!focused || g_cursorPos == dragStart) return vec2{0};
	return mouseDelta;
}

vec2 GestureDetector::Storage::getDragOffset() const {
	if (!focused) return vec2{0};
	return g_cursorPos - dragStart;
}

const vec2 &GestureDetector::Storage::getDragStartPos() const {
	return dragStart;
}

squi::GestureDetector::operator Child() const {
	child->state.properties["gestureDetector"] = Storage {
		.onEnter = onEnter,
		.onLeave = onLeave,
		.onClick = onClick,
		.onPress = onPress,
		.onRelease = onRelease,
		.onDrag = onDrag,
		.onUpdate = onUpdate,
	};
	auto &childFuncs = child->funcs();
	childFuncs.onUpdate.emplace(childFuncs.onUpdate.begin(), [](Widget &widget) {
		auto &storage = std::any_cast<Storage&>(widget.state.properties.at("gestureDetector"));
		storage.update(widget);
		if (storage.onUpdate) storage.onUpdate(widget, storage);
	});
	
	return child;
}
