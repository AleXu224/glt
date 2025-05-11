#include "gestureDetector.hpp"
#include "GLFW/glfw3.h"
#include "widget.hpp"
#include "window.hpp"
#include <memory>


using namespace squi;

bool GestureDetector::State::isKey(int key, int action, int mods) const {
	auto inputState = getInputState();
	if (!inputState) return false;
	if (!inputState->get().g_keys.contains(key)) return false;

	auto &keyInput = inputState->get().g_keys.at(key);
	return (keyInput.action == action && keyInput.mods == mods);
}

bool GestureDetector::State::isKeyPressedOrRepeat(int key, int mods) const {
	auto inputState = getInputState();
	if (!inputState) return false;

	if (!inputState->get().g_keys.contains(key)) return false;

	auto &keyInput = inputState->get().g_keys.at(key);
	return ((keyInput.action == GLFW_PRESS || keyInput.action == GLFW_REPEAT) && keyInput.mods == mods);
}

void GestureDetector::Storage::update(State &state) {
	auto inputStateOpt = state.getInputState();
	if (!inputStateOpt) return;
	auto &inputState = inputStateOpt->get();
	if (GestureDetector::canClick(*state.child.lock())) {
		state.scrollDelta = inputState.g_scrollDelta;

		if (!state.hovered && onEnter) onEnter(Event{*state.child.lock(), state});
		state.hovered = true;

		if (inputState.isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS) && !state.focusedOutside) {
			if (!state.focused) {
				state.dragStart = inputState.g_cursorPos;
				if (onPress) onPress({*state.child.lock(), state});
				if (onFocus) onFocus({*state.child.lock(), state});
			}
			state.focused = true;
			if (!state.active && onActive) onActive({*state.child.lock(), state});
			state.active = true;
		} else if (inputState.isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE)) {
			if (state.focused && !state.focusedOutside) {
				if (onClick) onClick({*state.child.lock(), state});
				if (onRelease) onRelease({*state.child.lock(), state});
			}
			if (state.focused && onFocusLoss) onFocusLoss({*state.child.lock(), state});
			state.focused = false;
			state.focusedOutside = false;
		}
	} else {
		state.scrollDelta = vec2{0};

		if (state.hovered && onLeave) onLeave({*state.child.lock(), state});
		state.hovered = false;

		if (inputState.isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS) && !state.focused) {
			state.focusedOutside = true;
			if (state.active && onInactive) onInactive({*state.child.lock(), state});
			state.active = false;
		} else if (inputState.isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE)) {
			if (state.focused && onFocusLoss) onFocusLoss({*state.child.lock(), state});
			state.focused = false;
			state.focusedOutside = false;
		}
	}

	if (state.focused && onDrag)
		onDrag({*state.child.lock(), state});

	if (state.active)
		state.textInput = inputState.g_textInput;
	else
		state.textInput.clear();
}

const vec2 &GestureDetector::State::getScroll() const {
	return scrollDelta;
}

vec2 GestureDetector::State::getDragDelta() const {
	auto inputStateOpt = getInputState();
	if (!inputStateOpt) return {};
	auto &inputState = inputStateOpt->get();
	if (!focused || inputState.g_cursorPos == dragStart) return vec2{0};
	return inputState.mouseDelta;
}

vec2 GestureDetector::State::getDragOffset() const {
	auto inputStateOpt = getInputState();
	if (!inputStateOpt) return {};
	auto &inputState = inputStateOpt->get();
	if (!focused) return vec2{0};
	return inputState.g_cursorPos - dragStart;
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
		.state{
			widget.weak_from_this(),
		},
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
		storage->update(storage->state);
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

std::optional<std::reference_wrapper<InputState>> squi::GestureDetector::State::getInputState() const {
	auto widget = child.lock();
	if (!widget) return {};
	return Window::of(widget).inputState;
}

vec2 squi::GestureDetector::State::getCursorPos() const {
	auto inputStateOpt = getInputState();
	if (!inputStateOpt) return {};
	auto &inputState = inputStateOpt->get();
	return inputState.g_cursorPos;
}

bool squi::GestureDetector::canClick(Widget &widget) {
	auto &inputState = Window::of(&widget).inputState;

	bool cursorInsideAnotherWidget = false;
	const bool cursorInsideWidget = widget.getRect().contains(inputState.g_cursorPos);
	const bool cursorInsideActiveArea = inputState.g_activeArea.back().contains(inputState.g_cursorPos);
	if (widget.flags.isInteractive && cursorInsideWidget && cursorInsideActiveArea) {
		for (auto &widgetRect: inputState.g_hitCheckRects) {
			if (widgetRect.contains(inputState.g_cursorPos)) {
				cursorInsideAnotherWidget = true;
				break;
			}
		}
	}

	return (widget.flags.isInteractive && inputState.g_cursorInside && !cursorInsideAnotherWidget && cursorInsideWidget && cursorInsideActiveArea);
}
