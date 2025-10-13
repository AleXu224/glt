#include "core/inputState.hpp"

#include "utils.hpp"
#include <GLFW/glfw3.h>


namespace squi::core {
	void InputState::setCursorPos(const vec2 &pos) {
		lastCursorPos = g_cursorPos;
		g_cursorPos = pos;
		mouseDelta += g_cursorPos - lastCursorPos;
	}

	void InputState::frameBegin() {
		for (const auto &[key, state]: g_keys) {
			g_keys_persistent[key] = state;
		}
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

	std::optional<KeyState> InputState::getKey(GestureKey key) const {
		if (!g_keys.contains(key)) return std::nullopt;

		return g_keys.at(key);
	}

	std::optional<KeyState> InputState::getKeyPressedOrRepeat(GestureKey key) const {
		if (!g_keys.contains(key)) return std::nullopt;

		const auto &keyInput = g_keys.at(key);
		if (keyInput.action == GestureAction::press || keyInput.action == GestureAction::repeat) return keyInput;

		return std::nullopt;
	}

	bool InputState::isKey(std::variant<GestureKey, GestureMouseKey> key, GestureAction action, GestureMod mods) const {
		if (!std::visit(
				utils::overloaded{
					[&](const GestureKey &key) {
						return g_keys.contains(key);
					},
					[&](const GestureMouseKey &key) {
						return g_mouseKeys.contains(key);
					},
				},
				key
			)) return false;

		const auto &keyInput = std::visit(
			utils::overloaded{
				[&](const GestureKey &key) {
					return g_keys.at(key);
				},
				[&](const GestureMouseKey &key) {
					return g_mouseKeys.at(key);
				},
			},
			key
		);
		return (keyInput.action == action && keyInput.mods == static_cast<int>(mods));
	}

	bool InputState::isKeyPressedOrRepeat(std::variant<GestureKey, GestureMouseKey> key, GestureMod mods) const {
		if (!std::visit(
				utils::overloaded{
					[&](const GestureKey &key) {
						return g_keys.contains(key);
					},
					[&](const GestureMouseKey &key) {
						return g_mouseKeys.contains(key);
					},
				},
				key
			)) return false;

		const auto &keyInput = std::visit(
			utils::overloaded{
				[&](const GestureKey &key) {
					return g_keys.at(key);
				},
				[&](const GestureMouseKey &key) {
					return g_mouseKeys.at(key);
				},
			},
			key
		);
		return ((keyInput.action == GestureAction::press || keyInput.action == GestureAction::repeat) && keyInput.mods == static_cast<int>(mods));
	}

	bool InputState::isKeyDown(GestureKey key) const {
		if (!g_keys_persistent.contains(key)) return false;

		const auto &keyInput = g_keys_persistent.at(key);
		return keyInput.action == GestureAction::press || keyInput.action == GestureAction::repeat;
	}

	void InputState::parseInput(const std::optional<InputTypes> &input) {
		if (!input) return;
		std::visit(
			utils::overloaded{
				[&](const StateChange &) {},
				[&](const CursorPosInput &input) {
					setCursorPos(vec2{input.xPos, input.yPos} / (g_dpi / 96.f));
				},
				[&](const CodepointInput &input) {
					g_textInput.append(1, input.character);
				},
				[&](const ScrollInput &input) {
					g_scrollDelta += vec2{input.xOffset, input.yOffset};
				},
				[&](const KeyInput &input) {
					if (!g_keys.contains(input.key))
						g_keys.insert({input.key, {.action = input.action, .mods = static_cast<int>(input.mods)}});
					else
						g_keys.at(input.key) = {.action = input.action, .mods = static_cast<int>(input.mods)};
				},
				[&](const MouseInput &input) {
					if (!g_mouseKeys.contains(input.button))
						g_mouseKeys.insert({input.button, {.action = input.action, .mods = static_cast<int>(input.mods)}});
					else
						g_mouseKeys.at(input.button) = {.action = input.action, .mods = static_cast<int>(input.mods)};
				},
				[&](const CursorEntered &input) {
					g_cursorInside = input.entered;
				},
			},
			input.value()
		);
	}
}// namespace squi::core