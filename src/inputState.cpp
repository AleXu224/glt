#include "inputState.hpp"

#include "GLFW/glfw3.h"
#include "window.hpp"


using namespace squi;

void InputState::setCursorPos(const vec2 &pos) {
	lastCursorPos = g_cursorPos;
	g_cursorPos = pos;
	mouseDelta += g_cursorPos - lastCursorPos;
}

void InputState::frameEnd() {
	lastCursorPos = g_cursorPos;
	mouseDelta = vec2{0};
	g_scrollDelta = vec2{0};
	g_textInput.clear();
	g_keys.clear();
}

const vec2 &InputState::getMousePos() const {
	return g_cursorPos;
}

vec2 InputState::getMouseDelta() const {
	return mouseDelta;
}

std::optional<KeyState> InputState::getKey(int key) const {
	if (!g_keys.contains(key)) return std::nullopt;

	return g_keys.at(key);
}

std::optional<KeyState> InputState::getKeyPressedOrRepeat(int key) const {
	if (!g_keys.contains(key)) return std::nullopt;

	const auto &keyInput = g_keys.at(key);
	if (keyInput.action == GLFW_PRESS || keyInput.action == GLFW_REPEAT) return keyInput;

	return std::nullopt;
}

bool InputState::isKey(int key, int action, int mods) const {
	if (!g_keys.contains(key)) return false;

	const auto &keyInput = g_keys.at(key);
	return (keyInput.action == action && keyInput.mods == mods);
}

bool InputState::isKeyPressedOrRepeat(int key, int mods) const {
	if (!g_keys.contains(key)) return false;

	const auto &keyInput = g_keys.at(key);
	return ((keyInput.action == GLFW_PRESS || keyInput.action == GLFW_REPEAT) && keyInput.mods == mods);
}

InputState &squi::InputState::of(Widget *widget) {
	return Window::of(widget).inputState;
}
