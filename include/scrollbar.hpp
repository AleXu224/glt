#pragma once

#include "chrono"
#include "memory"
#include "scrollable.hpp"
#include "widget.hpp"


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
			std::chrono::steady_clock::time_point lastHoverTime{};
		};

		operator Child() const;
	};
}// namespace squi