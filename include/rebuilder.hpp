#pragma once

#include "widget.hpp"

namespace squi {
	struct Rebuilder {
		// Args
		squi::Widget::Args widget{};
		VoidObservable rebuildEvent;
		std::function<Child()> buildFunc{};

		operator squi::Child() const;
	};
}// namespace squi