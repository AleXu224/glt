#pragma once

#include "inputQueue.hpp"
#include "optional"
#include "rect.hpp"
#include "unordered_map"
#include "vec2.hpp"
#include "vector"
#include "widgets/misc/gestureEnums.hpp"


namespace squi::core {
	struct KeyState {
		GestureAction action;
		int mods;
	};

	struct InputState {
		vec2 lastCursorPos{};
		vec2 mouseDelta{};
		vec2 g_cursorPos{0};
		std::unordered_map<GestureKey, KeyState> g_keys{};
		std::unordered_map<GestureMouseKey, KeyState> g_mouseKeys{};
		std::unordered_map<GestureKey, KeyState> g_keys_persistent{};
		std::unordered_map<GestureMouseKey, KeyState> g_mouseKeys_persistent{};
		std::string g_textInput{};
		vec2 g_scrollDelta{0};
		std::vector<Rect> g_hitCheckRects{};
		// The area that can be currently interacted with
		// Take a scrollable for example: the content that is scrolled away is not interactable
		std::vector<Rect> g_activeArea{};
		vec2 g_dpi{96};
		bool g_cursorInside{false};

		void parseInput(const std::optional<InputTypes> &input);

		void setCursorPos(const vec2 &pos);
		void frameBegin();
		void frameEnd();
		[[nodiscard]] const vec2 &getMousePos() const;
		// Get how much the cursor has moved since the last frame
		[[nodiscard]] vec2 getMouseDelta() const;

		[[nodiscard]] std::optional<KeyState> getKey(GestureKey key) const;
		[[nodiscard]] std::optional<KeyState> getKeyPressedOrRepeat(GestureKey key) const;
		[[nodiscard]] bool isKey(std::variant<GestureKey, GestureMouseKey> key, GestureAction action, GestureMod mods = GestureMod::none) const;
		[[nodiscard]] bool isKeyPressedOrRepeat(std::variant<GestureKey, GestureMouseKey> key, GestureMod mods = GestureMod::none) const;
		[[nodiscard]] bool isKeyDown(GestureKey key) const;
	};
}// namespace squi::core