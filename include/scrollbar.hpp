#ifndef SQUI_SCROLLBAR_HPP
#define SQUI_SCROLLBAR_HPP

#include "box.hpp"
#include "memory"
#include "widget.hpp"

namespace squi {
	struct Scrollbar {
		// Args
		Widget::Args widget;
		std::function<void(const float &scroll)> onScroll{};
		std::function<std::tuple<float, float, float>()> setScroll{};

		struct Storage {
			// Data
            float contentHeight = 200;
            float viewHeight = 100;
			float scroll = 0;
			float scrollDragStart = 0;
		};

		operator Child() const;
	};
}// namespace squi

#endif