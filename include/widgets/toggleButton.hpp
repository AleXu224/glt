#pragma once

#include "core/core.hpp"
#include "widgets/button.hpp"

namespace squi {
	struct ToggleButton : StatelessWidget {
		// Args
		Key key;
		bool disabled = false;
		bool active = false;
		std::function<void(bool)> onToggle;
		std::variant<std::string, Child> content = "Toggle";

		[[nodiscard]] Child build(const Element &) const {
			return Button{
				.theme = active ? Button::Theme::Accent() : Button::Theme::Standard(),
				.disabled = disabled,
				.onClick = [this]() {
					if (onToggle) onToggle(!active);
				},
				.content = content,
			};
		}
	};
}// namespace squi