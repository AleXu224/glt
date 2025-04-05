#pragma once

#include "button.hpp"
#include "widget.hpp"


namespace squi {
	struct IconButton {
		// Args
		squi::Widget::Args widget{};
		char32_t icon = U'\0';
		ButtonStyle style{};
		bool disabled = false;
		std::function<void(GestureDetector::Event)> onClick{};

		operator squi::Child() const;
	};
}// namespace squi