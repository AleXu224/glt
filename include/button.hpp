#ifndef SQUI_BUTTON_HPP
#define SQUI_BUTTON_HPP

#include "color.hpp"
#include "widget.hpp"

namespace squi {
	struct ButtonStyle {
		Color color{Color::HEX(0x60CDFFFF)};
		Color borderColor{Color::HEX(0xFFFFFF14)};
		float borderRadius{4.0f};
		float borderWidth{1.0f};
        Margin padding{0, 12, 0, 12};
		Color textColor{Color::HEX(0x000000FF)};
		float textSize{14.0f};
	};

	struct Button {
		// Args
		Widget::Args widget;
		std::string_view text{"Button"};
		ButtonStyle style{};

		struct Storage {
			// Data
            Color color;
            Color borderColor;
		};

		operator Child() const;
	};
}// namespace squi

#endif