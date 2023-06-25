#ifndef SQUI_NAVIGATIONVIEW_HPP
#define SQUI_NAVIGATIONVIEW_HPP

#include "widget.hpp"

namespace squi {
	struct NavigationView {
		// Args
		Widget::Args widget;

		struct Storage {
			// Data
		};

		operator Child() const;
	};
}// namespace squi

#endif