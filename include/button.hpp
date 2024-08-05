#pragma once

#include "align.hpp"
#include "box.hpp"
#include "color.hpp"
#include "gestureDetector.hpp"
#include "text.hpp"
#include "widget.hpp"
#include <functional>

namespace squi {
	struct ButtonStyle {
		Color color{0x60CDFFFF};
		Color colorHover{color * 0.9f};
		Color colorActive{color * 0.8f};
		Color colorDisabled{0xFFFFFF28};
		Color borderColor{0x00000024};
		Color borderColorHover{borderColor};
		Color borderColorActive{0xFFFFFF14};
		Color borderColorDisabled{0};
		Box::BorderPosition borderPosition = Box::BorderPosition::inset;
		float borderRadius{4.0f};
		float borderWidth{1.0f};
		Color textColor{0x000000FF};
		float textSize{14.0f};

		static ButtonStyle Accent() {
			return {};
		}

		static ButtonStyle Standard() {
			return ButtonStyle{
				.color{0xFFFFFF0F},
				.colorHover{0xFFFFFF15},
				.colorActive{0xFFFFFF12},
				.colorDisabled{0xFFFFFF12},
				.borderColor{0xFFFFFF18},
				.borderColorHover{0xFFFFFF18},
				.borderColorActive{0xFFFFFF12},
				.borderColorDisabled{0xFFFFFF12},
				.borderPosition = Box::BorderPosition::outset,
				.textColor{0xFFFFFFFF},
			};
		}

		static ButtonStyle Subtle() {
			return ButtonStyle{
				.color{0xFFFFFF00},
				.colorHover{0xFFFFFF0F},
				.colorActive{0xFFFFFF0B},
				.colorDisabled{0xFFFFFF00},
				.borderWidth = 0.0f,
				.textColor{0xFFFFFFFF},
			};
		}
	};

	struct Button {
		// Args
		Widget::Args widget{};
		std::variant<float, Size> width = Size::Shrink;
		std::variant<float, Size> height = Size::Shrink;
		Margin margin{};
		Margin padding = {6, 12, 6, 12};
		std::string_view text{"Button"};
		ButtonStyle style{};
		bool disabled = false;
		std::function<void(GestureDetector::Event)> onClick{};
		Child child = Align{.child = Text{.text = text}};

		operator Child() const;
	};
}// namespace squi