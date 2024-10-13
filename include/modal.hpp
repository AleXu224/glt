#pragma once

#include "observer.hpp"
#include "widget.hpp"

namespace squi {
	struct Modal {
		// Args
		squi::Widget::Args widget{};
		VoidObservable closeEvent{};
		Child child{};

		operator squi::Child() const;
	};
}// namespace squi