#ifndef SQUI_BUTTON_HPP
#define SQUI_BUTTON_HPP

#include "color.hpp"
#include "widget.hpp"
#include "box.hpp"

namespace squi {
	struct ButtonStyle {
		Color color{Color::HEX(0x60CDFFFF)};
		Color colorHover{color * 0.9f};
		Color colorActive{color * 0.8f};
		Color colorDisabled{Color::HEX(0xFFFFFF28)};
		Color borderColor{Color::HEX(0x00000024)};
		Color borderColorHover{borderColor};
		Color borderColorActive{Color::HEX(0xFFFFFF14)};
		Color borderColorDisabled{Color::HEX(0)};
		Box::BorderPosition borderPosition = Box::BorderPosition::inset;
		float borderRadius{4.0f};
		float borderWidth{1.0f};
		Margin padding{6, 12, 6, 12};
		Color textColor{Color::HEX(0x000000FF)};
		float textSize{14.0f};

		static ButtonStyle Accent() {
			return {};
		}

		static ButtonStyle Standard() {
			return ButtonStyle{
				.color{Color::HEX(0xFFFFFF0F)},
				.colorHover{Color::HEX(0xFFFFFF15)},
				.colorActive{Color::HEX(0xFFFFFF12)},
				.colorDisabled{Color::HEX(0xFFFFFF12)},
				.borderColor{Color::HEX(0xFFFFFF18)},
				.borderColorHover{Color::HEX(0xFFFFFF18)},
				.borderColorActive{Color::HEX(0xFFFFFF12)},
				.borderColorDisabled{Color::HEX(0xFFFFFF12)},
				.borderPosition = Box::BorderPosition::outset,
				.textColor{Color::HEX(0xFFFFFFFF)},
			};
		}
	};

	struct Button {
		// Args
		Widget::Args widget;
		std::string_view text{"Button"};
		ButtonStyle style{};
		bool disabled = false;

		struct Storage {
			// Data
			Color color;
			Color colorHover;
			Color colorActive;
			Color colorDisabled;
			Color borderColor;
			Color borderColorHover;
			Color borderColorActive;
			Color borderColorDisabled;
			bool disabled;
		};

		operator Child() const;
	};
}// namespace squi

#endif