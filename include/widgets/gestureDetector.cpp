#include "widgets/gestureDetector.hpp"

#include "core/app.hpp"
#include "inputPassthrough.hpp"
#include "utils.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>


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
		return (keyInput.action == action && keyInput.mods == mods.value);
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
		return ((keyInput.action == GestureAction::press || keyInput.action == GestureAction::repeat) && keyInput.mods == mods.value);
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

	Rect Gesture::DetectorRenderObject::getHitcheckRect() const {
		if (child) return child->getRect();
		return getRect();
	}

	std::optional<float> Gesture::DetectorRenderObject::getEffectiveDragThreshold() const {
		const auto &widget = *getWidgetAs<Gesture>();
		if (widget.dragThreshold) return *widget.dragThreshold;
		if (effectiveDragThreshold) return *effectiveDragThreshold;
		return std::nullopt;
	}

	void Gesture::DetectorRenderObject::update() {
		SingleChildRenderObject::update();
		auto *app = getApp();
		assert(app);
		auto &inputState = app->inputState;
		auto &widget = static_cast<Gesture &>(*element->widget);

		const auto hitIt = inputState.g_hitIndex.find(this);
		const bool inPath = hitIt != inputState.g_hitIndex.end();
		const HitEntry *hit = inPath ? &inputState.g_hitPath[hitIt->second] : nullptr;

		const bool hovered = inPath && hit->canHover;
		if (hovered != state.hovered) {
			state.hovered = hovered;
			if (hovered) {
				if (widget.onEnter) widget.onEnter(state);
			} else {
				if (widget.onLeave) widget.onLeave(state);
			}
		}
		state.scrollDelta = hovered && hit->canScroll ? inputState.g_scrollDelta : vec2{0};

		if (inputState.isKey(GestureMouseKey::left, GestureAction::press)) {
			if (inPath && !state.focusedOutside && hit->canFocus) {
				if (!state.focused) {
					state.dragStart = inputState.g_cursorPos;
					state.reachedDragThreshold = false;
					if (widget.onPress) widget.onPress(state);
					if (widget.onFocus) widget.onFocus(state);
				}
				state.focused = true;
				if (hit->canActivate) {
					if (!state.active && widget.onActive) widget.onActive(state);
					state.active = true;
				} else if (state.active && widget.onInactive) {
					widget.onInactive(state);
					state.active = false;
				}
				state.dragging = hit->canDrag;
			} else {
				if (!state.focused) {
					if (state.active && widget.onInactive) widget.onInactive(state);
					state.active = false;
					state.focusedOutside = true;
				}
			}
		} else if (inputState.isKey(GestureMouseKey::left, GestureAction::release)) {
			if (state.focused) {
				const bool canClick = inPath && hit->canClick && !state.reachedDragThreshold;
				if (canClick) {
					if (widget.onClick) widget.onClick(state);
					if (widget.onRelease) widget.onRelease(state);
				}
				if (widget.onFocusLoss) widget.onFocusLoss(state);
				state.focused = false;
				state.focusedOutside = false;
				state.dragging = false;
			} else {
				state.focusedOutside = false;
			}
		}

		if (state.focused) {
			if (const auto threshold = getEffectiveDragThreshold()) {
				const float dragDistance = (inputState.g_cursorPos - state.dragStart).length();
				if (dragDistance > *threshold) state.reachedDragThreshold = true;
			}
			if (widget.onDrag && state.dragging) widget.onDrag(state);
		}

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
		state.renderObject = this;
	}

	void Gesture::finalizeHitTest(InputState &inputState) {
		auto &path = inputState.g_hitPath;
		auto &index = inputState.g_hitIndex;
		index.clear();
		index.reserve(path.size());
		for (const auto &&[i, entry]: path | std::views::enumerate) {
			index[entry.renderObject] = i;
		}

		InputLevel grantedLevel = InputLevel::none;
		std::optional<float> activeDragThreshold{};
		for (auto it = path.rbegin(); it != path.rend(); ++it) {
			auto &entry = *it;
			auto *renderObject = entry.renderObject;
			if (auto *detector = dynamic_cast<DetectorRenderObject *>(renderObject)) {
				entry.grantLevel = grantedLevel;
				entry.effectiveDragThreshold = activeDragThreshold;
				detector->effectiveDragThreshold = activeDragThreshold;
				grantedLevel = std::max(grantedLevel, detector->getWidgetAs<Gesture>()->requirements);
			} else if (auto *flags = dynamic_cast<InputPassthrough::InputPassthroughRenderObject *>(renderObject)) {
				const auto *widget = flags->getWidgetAs<InputPassthrough>();
				if (widget->override) grantedLevel = *widget->override;
				grantedLevel = std::max(grantedLevel, widget->passthrough);
				if (widget->dragThreshold) activeDragThreshold = *widget->dragThreshold;
			}
		}

		bool hoverBlocked = false;
		bool scrollBlocked = false;
		bool focusBlocked = false;
		bool dragBlocked = false;
		bool clickBlocked = false;
		bool activateBlocked = false;
		for (auto &entry: path) {
			auto *detector = dynamic_cast<DetectorRenderObject *>(entry.renderObject);
			if (!detector) continue;
			entry.canHover = !hoverBlocked;
			entry.canScroll = !scrollBlocked;
			entry.canFocus = !focusBlocked;
			entry.canDrag = !dragBlocked;
			entry.canClick = !clickBlocked;
			entry.canActivate = !activateBlocked;
			if (entry.grantLevel < InputLevel::hover) hoverBlocked = true;
			if (entry.grantLevel < InputLevel::scroll) scrollBlocked = true;
			if (entry.grantLevel < InputLevel::focus) focusBlocked = true;
			if (entry.grantLevel < InputLevel::drag) dragBlocked = true;
			if (entry.grantLevel < InputLevel::click) clickBlocked = true;
			if (entry.grantLevel < InputLevel::activate) activateBlocked = true;
		}
	}
}// namespace squi