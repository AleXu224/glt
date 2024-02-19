#include "gestureDetector.hpp"
#include "GLFW/glfw3.h"
#include "widget.hpp"
#include <memory>
#include <optional>

using namespace squi;

vec2 GestureDetector::lastCursorPos{};
vec2 GestureDetector::mouseDelta{0};

vec2 GestureDetector::g_cursorPos{0};
std::unordered_map<int, KeyState> GestureDetector::g_keys{};
std::string GestureDetector::g_textInput{};
vec2 GestureDetector::g_scrollDelta{0};
std::vector<Rect> GestureDetector::g_hitCheckRects{};
std::vector<Rect> GestureDetector::g_activeArea{};
vec2 GestureDetector::g_dpi{96};
bool GestureDetector::g_cursorInside{false};

void GestureDetector::setCursorPos(const vec2 &pos) {
	lastCursorPos = g_cursorPos;
	g_cursorPos = pos;
	mouseDelta += g_cursorPos - lastCursorPos;
}

void GestureDetector::frameEnd() {
	lastCursorPos = g_cursorPos;
	mouseDelta = vec2{0};
	g_scrollDelta = vec2{0};
	g_textInput.clear();
	g_keys.clear();
}

std::optional<KeyState> GestureDetector::getKey(int key) {
	if (!g_keys.contains(key)) return std::nullopt;

	return g_keys.at(key);
}

std::optional<KeyState> GestureDetector::getKeyPressedOrRepeat(int key) {
	if (!g_keys.contains(key)) return std::nullopt;

	auto &keyInput = g_keys.at(key);
	if (keyInput.action == GLFW_PRESS || keyInput.action == GLFW_REPEAT) return keyInput;

	return std::nullopt;
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
	if (widget.flags.isInteractive && cursorInsideWidget && cursorInsideActiveArea) {
		for (auto &widgetRect: g_hitCheckRects) {
			if (widgetRect.contains(g_cursorPos)) {
				cursorInsideAnotherWidget = true;
				break;
			}
		}
	}

	if (widget.flags.isInteractive && g_cursorInside && !cursorInsideAnotherWidget && cursorInsideWidget && cursorInsideActiveArea) {
		state.scrollDelta = g_scrollDelta;

		if (!state.hovered && onEnter) onEnter(Event{widget, state});
		state.hovered = true;

		if (isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS) && !state.focusedOutside) {
			if (!state.focused) {
				state.dragStart = g_cursorPos;
				if (onPress) onPress({widget, state});
				if (onFocus) onFocus({widget, state});
			}
			state.focused = true;
			if (!state.active && onActive) onActive({widget, state});
			state.active = true;
		} else if (isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE)) {
			if (state.focused && !state.focusedOutside) {
				if (onClick) onClick({widget, state});
				if (onRelease) onRelease({widget, state});
			}
			if (state.focused && onFocusLoss) onFocusLoss({widget, state});
			state.focused = false;
			state.focusedOutside = false;
		}
	} else {
		state.scrollDelta = vec2{0};

		if (state.hovered && onLeave) onLeave({widget, state});
		state.hovered = false;

		if (isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS) && !state.focused) {
			state.focusedOutside = true;
			if (state.active && onInactive) onInactive({widget, state});
			state.active = false;
		} else if (isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE)) {
			if (state.focused && onFocusLoss) onFocusLoss({widget, state});
			state.focused = false;
			state.focusedOutside = false;
		}
	}

	if (state.active && onDrag)
		onDrag({widget, state});

	if (state.active)
		state.textInput = g_textInput;
	else
		state.textInput.clear();
}

const vec2 &GestureDetector::getMousePos() {
	return g_cursorPos;
}

vec2 GestureDetector::getMouseDelta() {
	return mouseDelta;
}

const vec2 &GestureDetector::State::getScroll() const {
	return scrollDelta;
}

vec2 GestureDetector::State::getDragDelta() const {
	if (!focused || g_cursorPos == dragStart) return vec2{0};
	return mouseDelta;
}

vec2 GestureDetector::State::getDragOffset() const {
	if (!focused) return vec2{0};
	return g_cursorPos - dragStart;
}

const vec2 &GestureDetector::State::getDragStartPos() const {
	return dragStart;
}

GestureDetector::operator Child() const {
	if (!child) return nullptr;

	mount(*child);

	return child;
}

GestureDetector::State &GestureDetector::mount(Widget &widget) const {
	auto storage = std::make_shared<Storage>(Storage{
		.onEnter = onEnter,
		.onLeave = onLeave,
		.onFocus = onFocus,
		.onFocusLoss = onFocusLoss,
		.onActive = onActive,
		.onInactive = onInactive,
		.onClick = onClick,
		.onPress = onPress,
		.onRelease = onRelease,
		.onDrag = onDrag,
		.onUpdate = onUpdate,
	});
	widget.funcs().onUpdate.emplace(widget.funcs().onUpdate.begin(), [storage](Widget &widget) mutable {
		storage->update(widget);
		if (storage->onUpdate) storage->onUpdate({widget, storage->state});
		if (storage->state.forceActive) {
			if (!storage->state.active && storage->onActive) storage->onActive({widget, storage->state});
			storage->state.active = true;
			storage->state.forceActive = false;
		}
		if (storage->state.forceInactive) {
			if (storage->state.active && storage->onInactive) storage->onInactive({widget, storage->state});
			storage->state.active = false;
			storage->state.forceInactive = false;
		}
	});
	return storage->state;
}

void squi::GestureDetector::State::setActive() {
	forceActive = true;
}

void squi::GestureDetector::State::setInactive() {
	forceInactive = true;
}
