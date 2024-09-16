#pragma once

#include "optional"
#include "rect.hpp"
#include "unordered_map"
#include "vec2.hpp"
#include "vector"


namespace squi {
	class Widget;

	struct KeyState {
		int action;
		int mods;
	};

	struct InputState {
		vec2 lastCursorPos{};
		vec2 mouseDelta{};
		vec2 g_cursorPos{0};
		std::unordered_map<int, KeyState> g_keys{};
		std::string g_textInput{};
		vec2 g_scrollDelta{0};
		std::vector<Rect> g_hitCheckRects{};
		std::vector<Rect> g_activeArea{};
		vec2 g_dpi{96};
		bool g_cursorInside{false};

		void setCursorPos(const vec2 &pos);
		void frameEnd();
		[[nodiscard]] const vec2 &getMousePos() const;
		// Get how much the cursor has moved since the last frame
		[[nodiscard]] vec2 getMouseDelta() const;

		[[nodiscard]] std::optional<KeyState> getKey(int key) const;
		[[nodiscard]] std::optional<KeyState> getKeyPressedOrRepeat(int key) const;
		[[nodiscard]] bool isKey(int key, int action, int mods = 0) const;
		[[nodiscard]] bool isKeyPressedOrRepeat(int key, int mods = 0) const;

		[[nodiscard]] static InputState &of(Widget *widget);
	};
}// namespace squi