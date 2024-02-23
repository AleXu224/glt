#pragma once

#include "rect.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include <optional>

namespace squi {
	struct KeyState {
		int action;
		int mods;
	};
	struct GestureDetector {
		// Args
		struct Storage;
		struct State {
			// Data
			friend Storage;
			// Wether the cursor is directly above the widget, without a stacked widget being on top
			bool hovered = false;
			// Wether the left mouse button is currently being held on top of the widget
			bool focused = false;
			// Wether the left mouse button started being held outside of the widget
			bool focusedOutside = false;
			// Indicates if the widget has been activated by a click and no other widget has been focused since
			bool active = false;
			std::string textInput{};

			// Get how much the scroll has moved in the last frame
			[[nodiscard]] const vec2 &getScroll() const;
			// Get how much the cursor has moved since last update
			[[nodiscard]] vec2 getDragDelta() const;
			// Get how much the cursor has moved since it began dragging
			[[nodiscard]] vec2 getDragOffset() const;
			// Get the location of where the drag began
			[[nodiscard]] const vec2 &getDragStartPos() const;

			void setActive();
			void setInactive();

		private:
			friend GestureDetector;
			bool forceActive = false;
			bool forceInactive = false;
			vec2 scrollDelta{};
			vec2 dragStart{};
		};

		struct Event {
			Widget &widget;
			State &state;
		};

		static void setActive(Event event);
		static void setInactive(Event event);

		std::function<void(Event)> onEnter{};
		std::function<void(Event)> onLeave{};
		std::function<void(Event)> onFocus{};
		std::function<void(Event)> onFocusLoss{};
		std::function<void(Event)> onActive{};
		std::function<void(Event)> onInactive{};
		std::function<void(Event)> onClick{};
		std::function<void(Event)> onPress{};
		std::function<void(Event)> onRelease{};
		std::function<void(Event)> onDrag{};
		std::function<void(Event)> onUpdate{};
		/**
		 * @brief Called on initalization. Use this to store the state of the GestureDetector.
		 */
		Child child{};

	private:
		static vec2 lastCursorPos;
		static vec2 mouseDelta;

	public:
		static vec2 g_cursorPos;
		static std::unordered_map<int, KeyState> g_keys;
		static std::string g_textInput;
		static vec2 g_scrollDelta;
		static std::vector<Rect> g_hitCheckRects;
		static std::vector<Rect> g_activeArea;
		static vec2 g_dpi;
		static bool g_cursorInside;

		static void setCursorPos(const vec2 &pos);
		static void frameEnd();
		[[nodiscard]] static const vec2 &getMousePos();
		// Get how much the cursor has moved since the last frame
		[[nodiscard]] static vec2 getMouseDelta();

		[[nodiscard]] static std::optional<KeyState> getKey(int key);
		[[nodiscard]] static std::optional<KeyState> getKeyPressedOrRepeat(int key);
		[[nodiscard]] static bool isKey(int key, int action, int mods = 0);
		[[nodiscard]] static bool isKeyPressedOrRepeat(int key, int mods = 0);
		[[nodiscard]] static bool canClick(Widget &widget);

		struct Storage {
			// Data
			State state{};
			std::function<void(Event)> onEnter{};
			std::function<void(Event)> onLeave{};
			std::function<void(Event)> onFocus{};
			std::function<void(Event)> onFocusLoss{};
			std::function<void(Event)> onActive{};
			std::function<void(Event)> onInactive{};
			std::function<void(Event)> onClick{};
			std::function<void(Event)> onPress{};
			std::function<void(Event)> onRelease{};
			std::function<void(Event)> onDrag{};
			std::function<void(Event)> onUpdate{};

			void update(Widget &widget);
		};

		operator Child() const;

		[[maybe_unused]] State &mount(Widget &widget) const;
	};
}// namespace squi