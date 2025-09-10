#pragma once

#include "core/core.hpp"


namespace squi {
	struct GestureDetector : RenderObjectWidget {
		struct State;

		// Args
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

		Child child{};

		struct State {
			// Wether the cursor is directly above the widget, without a stacked widget being on top
			bool hovered = false;
			// Wether the left mouse button is currently being held on top of the widget
			bool focused = false;
			// Wether the left mouse button started being held outside of the widget
			bool focusedOutside = false;
			// Indicates if the widget has been activated by a click and no other widget has been focused since
			bool active = false;
			std::string textInput{};

			vec2 scrollDelta{};
			vec2 dragStart{};
		};

		struct Element : SingleChildRenderObjectElement {
			using SingleChildRenderObjectElement::SingleChildRenderObjectElement;

			Child build() override {
				if (auto gestureDetectorWidget = std::static_pointer_cast<GestureDetector>(widget)) {
					return gestureDetectorWidget->child;
				}
				return nullptr;
			}
		};

		struct DetectorRenderObject : SingleChildRenderObject {
			State state{};

			void update();
			bool canClick() const;
		};

		std::shared_ptr<RenderObject> createRenderObject() const {
			return std::make_shared<DetectorRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const {
			// Update render object properties here
		}

		Args getArgs() const {
			return {
				.width = Size::Wrap,
				.height = Size::Wrap
			};
		}
	};

}// namespace squi