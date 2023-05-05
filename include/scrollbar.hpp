#ifndef SQUI_SCROLLBAR_HPP
#define SQUI_SCROLLBAR_HPP

#include "box.hpp"
#include "gestureDetector.hpp"
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
			std::chrono::steady_clock::time_point lastHoverTime = std::chrono::steady_clock::now();
			std::shared_ptr<GestureDetector::Storage> gd{};
			std::shared_ptr<GestureDetector::Storage> containerGd{};
		};

		operator Child() const;
	};
}// namespace squi

#endif