#pragma once

#include "scrollable.hpp"
#include "widget.hpp"


namespace squi {
	struct ScrollableFrame {
		// Args
		Widget::Args widget{};
		Widget::Args scrollableWidget{};
		Scrollable::Alignment alignment{Scrollable::Alignment::begin};
		Scrollable::Direction direction{Scrollable::Direction::vertical};
		float spacing{0.0f};
		Children children{};

		struct Storage {
			// Data
			float contentHeight = 200;
			float viewHeight = 100;
			float scroll = 0;
		};

		operator Child() const;
	};
}// namespace squi