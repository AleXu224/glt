#pragma once

#include "core/animated.hpp"
#include "core/core.hpp"
#include "widgets/button.hpp"
#include "widgets/contextMenu.hpp"

namespace squi {
	struct DropdownButton : StatefulWidget {
		// Args
		Key key;
		std::string text;
		decltype(ContextMenu::items) items;
		Button::Theme theme = Button::Theme::Standard();

		struct State : WidgetState<DropdownButton> {
			vec2 contextMenuPosition;
			Button::ButtonStatus status = Button::ButtonStatus::resting;
			Animated<float> chevronRotation;

			void initState() override {
				chevronRotation.duration = 200ms;
				chevronRotation.mount(this);
			}

			Child build(const Element &element) override;
		};
	};
}// namespace squi