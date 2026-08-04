#pragma once

#include "core/core.hpp"
#include "core/inputState.hpp"
#include "misc/gestureEnums.hpp"

namespace squi {

	struct Gesture : RenderObjectWidget {
		struct State;

		// Args
		Key key;
		Args widget;
		std::function<void(const State &)> onEnter{};
		std::function<void(const State &)> onLeave{};
		std::function<void(const State &)> onFocus{};
		std::function<void(const State &)> onFocusLoss{};
		std::function<void(const State &)> onActive{};
		std::function<void(const State &)> onInactive{};
		std::function<void(const State &)> onClick{};
		std::function<void(const State &)> onPress{};
		std::function<void(const State &)> onRelease{};
		std::function<void(const State &)> onDrag{};
		std::function<void(const State &)> onUpdate{};

		InputLevel requirements = InputLevel::scroll;
		std::optional<float> dragThreshold{};

		Child child{};

		struct DetectorRenderObject;
		struct State {
			const InputState *inputState = nullptr;
			// Wether the cursor is directly above the widget, without a stacked widget being on top
			bool hovered = false;
			// Wether the left mouse button is currently being held on top of the widget
			bool focused = false;
			// Wether the left mouse button started being held outside of the widget
			bool focusedOutside = false;
			// Indicates if the widget has been activated by a click and no other widget has been focused since
			bool active = false;
			// Wether this gesture is currently being dragged
			bool dragging = false;
			// Wether the current press session has moved past the drag threshold and clicks are going to be ignored
			bool reachedDragThreshold = false;
			std::string textInput{};

			vec2 scrollDelta{};
			vec2 dragStart{};
			DetectorRenderObject *renderObject = nullptr;

			// Get how much the scroll has moved in the last frame
			[[nodiscard]] const vec2 &getScroll() const;
			// Get how much the cursor has moved since last update
			[[nodiscard]] vec2 getDragDelta() const;
			// Get how much the cursor has moved since it began dragging
			[[nodiscard]] vec2 getDragOffset() const;
			// Get the location of where the drag began
			[[nodiscard]] const vec2 &getDragStartPos() const;
			[[nodiscard]] bool isKey(std::variant<GestureKey, GestureMouseKey> key, GestureAction action, GestureMod mods = GestureMod::none) const;
			[[nodiscard]] bool isKeyPressedOrRepeat(std::variant<GestureKey, GestureMouseKey> key, GestureMod mods = GestureMod::none) const;
			[[nodiscard]] vec2 getCursorPos() const;
		};

		struct Element : SingleChildRenderObjectElement {
			using SingleChildRenderObjectElement::SingleChildRenderObjectElement;

			Child build() override {
				if (auto gestureDetectorWidget = std::static_pointer_cast<Gesture>(widget)) {
					return gestureDetectorWidget->child;
				}
				return nullptr;
			}
		};

		struct DetectorRenderObject : SingleChildRenderObject {
			State state{};
			std::optional<float> effectiveDragThreshold{};

			void update() override;
			void init() override;
			[[nodiscard]] Rect getHitcheckRect() const override;
			[[nodiscard]] std::optional<float> getEffectiveDragThreshold() const;
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<DetectorRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const {
			// Update render object properties here
		}

		static void finalizeHitTest(squi::core::InputState &inputState);

		[[nodiscard]] Args getArgs() const {
			auto ret = widget;
			ret.width = ret.width.value_or(Size::Wrap);
			ret.height = ret.height.value_or(Size::Wrap);
			return ret;
		}
	};

}// namespace squi