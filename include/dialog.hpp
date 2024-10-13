#pragma once

#include "observer.hpp"
#include "widget.hpp"

namespace squi {
	struct Dialog {
		// Args
		squi::Widget::Args widget{};
		VoidObservable closeEvent{};
        std::string_view title;
		Child content{};
        Children buttons{};

		operator squi::Child() const;
	};
}// namespace squi