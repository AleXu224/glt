#pragma once

#include "core/core.hpp"
#include "widgets/button.hpp"

namespace squi {
	struct IconButton : StatefulWidget {
		// Args
		Key key;
		Args widget;
		char32_t icon = 0xef4a;
		Button::Theme theme = Button::Theme::Standard();
		std::function<void()> onClick{};

		struct State : WidgetState<IconButton> {
			Button::ButtonStatus status = Button::ButtonStatus::resting;

			Child build(const Element &element) override;
		};
	};
}// namespace squi