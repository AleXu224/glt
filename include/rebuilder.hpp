#pragma once

#include "widget.hpp"

namespace squi {
	struct Rebuilder {
		// Args
		squi::Widget::Args widget{};
		VoidObservable rebuildEvent;
		std::function<void()> onRebuild{};
		std::function<Child()> buildFunc{};

		operator squi::Child() const;
	};
}// namespace squi