#pragma once

#include "core/core.hpp"
#include "widgets/button.hpp"

namespace squi {
	struct ToggleButton : StatelessWidget {
		// Args
		Key key;
		Args widget{};
		bool disabled = false;
		bool active = false;
		std::function<void(bool)> onToggle;
		std::variant<std::string, Child> child = "Toggle";

		[[nodiscard]] Child build(const Element &) const {
			return Button{
				.widget = widget,
				.theme = active ? Button::Theme::Accent() : Button::Theme::Standard(),
				.disabled = disabled,
				.onClick = [this]() {
					if (onToggle) onToggle(!active);
				},
				.child = child,
			};
		}
	};
}// namespace squi