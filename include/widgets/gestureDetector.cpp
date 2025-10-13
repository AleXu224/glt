#include "widgets/gestureDetector.hpp"

#include "core/app.hpp"
#include "utils.hpp"
#include <GLFW/glfw3.h>


namespace squi {

	bool Gesture::State::isKey(std::variant<GestureKey, GestureMouseKey> key, GestureAction action, GestureMod mods) const {
		if (!inputState) return false;
		bool found = std::visit(
			utils::overloaded{
				[&](const GestureKey &key) {
					return inputState->g_keys.contains(key);
				},
				[&](const GestureMouseKey &key) {
					return inputState->g_mouseKeys.contains(key);
				},
			},
			key
		);
		if (!found) return false;

		const auto &keyInput = std::visit(
			utils::overloaded{
				[&](const GestureKey &key) {
					return inputState->g_keys.at(key);
				},
				[&](const GestureMouseKey &key) {
					return inputState->g_mouseKeys.at(key);
				},
			},
			key
		);
		return (keyInput.action == action && keyInput.mods == static_cast<int>(mods));
	}

	bool Gesture::State::isKeyPressedOrRepeat(std::variant<GestureKey, GestureMouseKey> key, GestureMod mods) const {
		if (!inputState) return false;

		if (!std::visit(
				utils::overloaded{
					[&](const GestureKey &key) {
						return inputState->g_keys.contains(key);
					},
					[&](const GestureMouseKey &key) {
						return inputState->g_mouseKeys.contains(key);
					},
				},
				key
			)) return false;

		const auto &keyInput = std::visit(
			utils::overloaded{
				[&](const GestureKey &key) {
					return inputState->g_keys.at(key);
				},
				[&](const GestureMouseKey &key) {
					return inputState->g_mouseKeys.at(key);
				},
			},
			key
		);
		return ((keyInput.action == GestureAction::press || keyInput.action == GestureAction::repeat) && keyInput.mods == static_cast<int>(mods));
	}

	const vec2 &Gesture::State::getScroll() const {
		return scrollDelta;
	}

	vec2 Gesture::State::getDragDelta() const {
		if (!inputState) return {};
		if (!focused || inputState->g_cursorPos == dragStart) return vec2{0};
		return inputState->mouseDelta;
	}

	vec2 Gesture::State::getDragOffset() const {
		if (!inputState) return {};
		if (!focused) return vec2{0};
		return inputState->g_cursorPos - dragStart;
	}

	const vec2 &Gesture::State::getDragStartPos() const {
		return dragStart;
	}

	vec2 squi::Gesture::State::getCursorPos() const {
		if (!inputState) return {};
		return inputState->g_cursorPos;
	}

	void Gesture::DetectorRenderObject::update() {
		auto *app = getApp();
		assert(app);
		auto &inputState = app->inputState;
		auto &widget = static_cast<Gesture &>(*element->widget);

		if (canClick()) {
			state.scrollDelta = inputState.g_scrollDelta;

			if (!state.hovered && widget.onEnter) widget.onEnter(state);
			state.hovered = true;

			if (inputState.isKey(GestureMouseKey::left, GestureAction::press) && !state.focusedOutside) {
				if (!state.focused) {
					state.dragStart = inputState.g_cursorPos;
					if (widget.onPress) widget.onPress(state);
					if (widget.onFocus) widget.onFocus(state);
				}
				state.focused = true;
				if (!state.active && widget.onActive) widget.onActive(state);
				state.active = true;
			} else if (inputState.isKey(GestureMouseKey::left, GestureAction::release)) {
				if (state.focused && !state.focusedOutside) {
					if (widget.onClick) widget.onClick(state);
					if (widget.onRelease) widget.onRelease(state);
				}
				if (state.focused && widget.onFocusLoss) widget.onFocusLoss(state);
				state.focused = false;
				state.focusedOutside = false;
			}
		} else {
			state.scrollDelta = vec2{0};

			if (state.hovered && widget.onLeave) widget.onLeave(state);
			state.hovered = false;

			if (inputState.isKey(GestureMouseKey::left, GestureAction::press) && !state.focused) {
				state.focusedOutside = true;
				if (state.active && widget.onInactive) widget.onInactive(state);
				state.active = false;
			} else if (inputState.isKey(GestureMouseKey::left, GestureAction::release)) {
				if (state.focused && widget.onFocusLoss) widget.onFocusLoss(state);
				state.focused = false;
				state.focusedOutside = false;
			}
		}

		if (state.focused && widget.onDrag)
			widget.onDrag(state);

		if (state.active)
			state.textInput = inputState.g_textInput;
		else
			state.textInput.clear();

		if (widget.onUpdate) widget.onUpdate(state);
	}

	void Gesture::DetectorRenderObject::init() {
		auto *app = this->getApp();
		if (!app) return;

		state.inputState = &app->inputState;
	}

	bool Gesture::DetectorRenderObject::canClick() const {
		if (!child) return false;

		auto *app = getApp();
		if (!app) return false;

		auto &inputState = app->inputState;

		bool cursorInsideAnotherWidget = false;
		const bool cursorInsideWidget = child->getRect().contains(inputState.g_cursorPos);
		const bool cursorInsideActiveArea = inputState.g_activeArea.back().contains(inputState.g_cursorPos);
		if (cursorInsideWidget && cursorInsideActiveArea) {
			for (auto &widgetRect: inputState.g_hitCheckRects) {
				if (widgetRect.contains(inputState.g_cursorPos)) {
					cursorInsideAnotherWidget = true;
					break;
				}
			}
		}

		return (inputState.g_cursorInside && !cursorInsideAnotherWidget && cursorInsideWidget && cursorInsideActiveArea);
	}
}// namespace squi