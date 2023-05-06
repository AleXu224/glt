#ifndef SQUI_GESTUREDETECTOR_HPP
#define SQUI_GESTUREDETECTOR_HPP

#include "rect.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include <memory>

namespace squi {
	struct KeyState {
		int action;
		int mods;
	};
	struct GestureDetector {
		// Args
		struct Storage;
		std::function<void(Widget &, Storage &)> onEnter{};
		std::function<void(Widget &, Storage &)> onLeave{};
		std::function<void(Widget &, Storage &)> onClick{};
		std::function<void(Widget &, Storage &)> onPress{};
		std::function<void(Widget &, Storage &)> onRelease{};
		std::function<void(Widget &, Storage &)> onDrag{};
		std::function<void(Widget &, Storage &)> onUpdate{};
		/**
		 * @brief Called on initalization. Use this to store the state of the GestureDetector.
		 */
		std::function<void(Widget &, std::shared_ptr<Storage>)> getState{};
		Child child{};

	private:
		static vec2 lastCursorPos;
		static vec2 mouseDelta;

	public:
		static vec2 g_cursorPos;
		static std::unordered_map<int, KeyState> g_keys;
		static unsigned char g_textInput;
		static vec2 g_scrollDelta;
		static std::vector<Rect> g_hitCheckRects;
		static std::vector<Rect> g_activeArea;
		static vec2 g_dpi;
		static bool g_cursorInside;

		static void setCursorPos(const vec2 &pos);
		[[nodiscard]] static const vec2 &getMousePos();
		// Get how much the cursor has moved since the last frame
		[[nodiscard]] static vec2 getMouseDelta();

		[[nodiscard]] static bool isKey(int key, int action, int mods = 0);
		[[nodiscard]] static bool isKeyPressedOrRepeat(int key, int mods = 0);

		struct Storage {
			// Data
			vec2 scrollDelta{};
			vec2 dragStart{};
			// Wether the cursor is directly above the widget, without a stacked widget being on top
			bool hovered = false;
			// Wether the left mouse button is currently being held on top of the widget
			bool focused = false;
			// Wether the left mouse button started being held outside of the widget
			bool focusedOutside = false;
			// Indicates if the widget has been activated by a click and no other widget has been focused since
			bool active = false;
			unsigned int charInput = 0;
			std::function<void(Widget &, Storage &)> onEnter{};
			std::function<void(Widget &, Storage &)> onLeave{};
			std::function<void(Widget &, Storage &)> onClick{};
			std::function<void(Widget &, Storage &)> onPress{};
			std::function<void(Widget &, Storage &)> onRelease{};
			std::function<void(Widget &, Storage &)> onDrag{};
			std::function<void(Widget &, Storage &)> onUpdate{};

			void update(Widget &widget);

			// Get how much the scroll has moved in the last frame
			[[nodiscard]] const vec2 &getScroll() const;
			// Get how much the cursor has moved since last update
			[[nodiscard]] vec2 getDragDelta() const;
			// Get how much the cursor has moved since it began dragging
			[[nodiscard]] vec2 getDragOffset() const;
			// Get the location of where the drag began
			[[nodiscard]] const vec2 &getDragStartPos() const;
		};
	
		operator Child() const;

		std::shared_ptr<Storage> initializeFor(Widget &widget) const;
	};
}// namespace squi

#endif