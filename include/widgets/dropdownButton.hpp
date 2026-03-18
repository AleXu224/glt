#pragma once

#include "core/animated.hpp"
#include "core/core.hpp"
#include "widgets/button.hpp"
#include "widgets/contextMenu.hpp"

namespace squi {
	struct DropdownButton : StatefulWidget {
		// Args
		Key key;
		Args widget{};
		Button::Theme theme = Button::Theme::Standard();
		bool disabled = false;
		std::string text = "Dropdown";
		decltype(ContextMenu::items) items;

		struct State : WidgetState<DropdownButton> {
			vec2 contextMenuPosition;
			Button::ButtonStatus status = Button::ButtonStatus::resting;
			Animated<float> chevronRotation;
			Key dropdownKey;

			void initState() override {
				chevronRotation.duration = 200ms;
				chevronRotation.mount(this);
				dropdownKey = std::make_shared<IndexKey>(element->id);
			}

			Child build(const Element &element) override;
		};
	};
}// namespace squi