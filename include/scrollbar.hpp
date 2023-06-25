#ifndef SQUI_SCROLLBAR_HPP
#define SQUI_SCROLLBAR_HPP

#include "box.hpp"
#include "gestureDetector.hpp"
#include "memory"
#include "scrollable.hpp"
#include "widget.hpp"
#include "chrono"

namespace squi {
	struct Scrollbar {
		// Args
		Widget::Args widget;
		std::shared_ptr<Scrollable::Controller> controller{std::make_shared<Scrollable::Controller>()};

		struct Storage {
			// Data
			float scroll = 0;
			float scrollDragStart = 0;
			std::shared_ptr<Scrollable::Controller> controller;
			std::chrono::steady_clock::time_point lastHoverTime = std::chrono::steady_clock::now();
		};

		operator Child() const;
	};
}// namespace squi

#endif