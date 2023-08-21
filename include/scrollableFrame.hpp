#pragma once

#include "column.hpp"
#include "widget.hpp"


namespace squi {
	struct ScrollableFrame {
		// Args
		Widget::Args widget;
		Widget::Args scrollableWidget;
		Column::Alignment alignment{Column::Alignment::left};
		float spacing{0.0f};
		Children children;

		struct Storage {
			// Data
			float contentHeight = 200;
			float viewHeight = 100;
			float scroll = 0;
		};

		operator Child() const;
	};
}// namespace squi