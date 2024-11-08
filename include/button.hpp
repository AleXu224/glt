#pragma once

#include "box.hpp"
#include "color.hpp"
#include "cstdint"
#include "gestureDetector.hpp"
#include "widgetArgs.hpp"
#include <functional>


namespace squi {
	struct ButtonStyle {
		Color color{0x60CDFFFF};
		Color colorHover{color * 0.9f};
		Color colorActive{color * 0.8f};
		Color colorDisabled{0xFFFFFF28};
		Color borderColor{Color::css(0xFFFFFF, 0.08f)};
		Color borderColorHover{borderColor};
		Color borderColorActive{0xFFFFFF14};
		Color borderColorDisabled{0};
		Box::BorderPosition borderPosition = Box::BorderPosition::inset;
		float borderRadius{4.0f};
		float borderWidth{1.0f};
		Color textColor{0x000000FF};
		Color textColorHover{0x000000FF};
		Color textColorActive{0.f, 0.f, 0.f, 0.5f};
		Color textColorDisabled{1.f, 1.f, 1.f, 0.5f};
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
				.textColorHover{0xFFFFFFFF},
				.textColorActive{1.f, 1.f, 1.f, 0.78f},
				.textColorDisabled{1.f, 1.f, 1.f, 0.36},
			};
		}

		static ButtonStyle Subtle() {
			return ButtonStyle{
				.color{0xFFFFFF00},
				.colorHover{0xFFFFFF0F},
				.colorActive{0xFFFFFF0B},
				.colorDisabled{0xFFFFFF00},
				.borderWidth = 0.0f,
				.textColorHover{0xFFFFFFFF},
				.textColorActive{1.f, 1.f, 1.f, 0.78f},
				.textColorDisabled{1.f, 1.f, 1.f, 0.36},
			};
		}
	};

	enum class ButtonState : uint8_t {
		resting,
		hovered,
		active,
		disabled,
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
		Child child{};

		operator Child() const;
	};
}// namespace squi