#include "widgets/gestureDetector.hpp"

#include "core/app.hpp"
#include <GLFW/glfw3.h>

namespace squi {
	const vec2 &GestureDetector::State::getScroll() const {
		return scrollDelta;
	}

	vec2 GestureDetector::State::getDragDelta() const {
		if (!inputState) return {};
		if (!focused || inputState->g_cursorPos == dragStart) return vec2{0};
		return inputState->mouseDelta;
	}

	vec2 GestureDetector::State::getDragOffset() const {
		if (!inputState) return {};
		if (!focused) return vec2{0};
		return inputState->g_cursorPos - dragStart;
	}

	const vec2 &GestureDetector::State::getDragStartPos() const {
		return dragStart;
	}

	vec2 squi::GestureDetector::State::getCursorPos() const {
		if (!inputState) return {};
		return inputState->g_cursorPos;
	}

	void GestureDetector::DetectorRenderObject::update() {
		auto app = getApp();
		assert(app);
		auto &inputState = app->inputState;
		auto &widget = static_cast<GestureDetector &>(*element->widget);

		if (canClick()) {
			state.scrollDelta = inputState.g_scrollDelta;

			if (!state.hovered && widget.onEnter) widget.onEnter(state);
			state.hovered = true;

			if (inputState.isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS) && !state.focusedOutside) {
				if (!state.focused) {
					state.dragStart = inputState.g_cursorPos;
					if (widget.onPress) widget.onPress(state);
					if (widget.onFocus) widget.onFocus(state);
				}
				state.focused = true;
				if (!state.active && widget.onActive) widget.onActive(state);
				state.active = true;
			} else if (inputState.isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE)) {
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

			if (inputState.isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS) && !state.focused) {
				state.focusedOutside = true;
				if (state.active && widget.onInactive) widget.onInactive(state);
				state.active = false;
			} else if (inputState.isKey(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE)) {
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

	void GestureDetector::DetectorRenderObject::init() {
		auto *app = this->getApp();
		if (!app) return;

		state.inputState = &app->inputState;
	}

	bool GestureDetector::DetectorRenderObject::canClick() const {
		auto app = getApp();
		if (!app) return false;

		auto &inputState = app->inputState;

		bool cursorInsideAnotherWidget = false;
		const bool cursorInsideWidget = getRect().contains(inputState.g_cursorPos);
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