#pragma once

#include "box.hpp"
#include "color.hpp"
#include "cstdint"
#include "gestureDetector.hpp"
#include "observer.hpp"
#include "stateInfo.hpp"
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
		BorderRadius borderRadius{4.0f};
		BorderWidth borderWidth{1.0f};
		Color textColor{0x000000FF};
		Color textColorHover{0x000000FF};
		Color textColorActive{0.f, 0.f, 0.f, 0.5f};
		Color textColorDisabled{1.f, 1.f, 1.f, 0.5f};
		float textSize{14.0f};

		static ButtonStyle Accent();

		static ButtonStyle Standard();

		static ButtonStyle Subtle();

		bool operator==(const ButtonStyle &other) const {
			return color == other.color
				&& colorHover == other.colorHover
				&& colorActive == other.colorActive
				&& colorDisabled == other.colorDisabled
				&& borderColor == other.borderColor
				&& borderColorHover == other.borderColorHover
				&& borderColorActive == other.borderColorActive
				&& borderColorDisabled == other.borderColorDisabled
				&& borderPosition == other.borderPosition
				&& borderRadius == other.borderRadius
				&& borderWidth == other.borderWidth
				&& textColor == other.textColor
				&& textColorHover == other.textColorHover
				&& textColorActive == other.textColorActive
				&& textColorDisabled == other.textColorDisabled
				&& textSize == other.textSize;
		}
		bool operator!=(const ButtonStyle &other) const { return !(*this == other); }
	};

	enum class ButtonState : uint8_t {
		resting,
		hovered,
		active,
		disabled,
	};

	struct Button {
		struct State {
			static inline StateInfo<Stateful<ButtonStyle, StateImpact::RedrawNeeded>> style{.name = "style"};
			static inline StateInfo<bool> disabled{.name = "disabled"};
			static inline StateInfo<ButtonState> state{.name = "state"};
			static inline StateInfo<Observable<ButtonState>> stateEvent{.name = "stateEvent"};
			static inline StateInfo<Observable<std::string>> updateText{.name = "updateText"};
		};

		// Args
		Widget::Args widget{};
		std::string_view text{"Button"};
		ButtonStyle style = ButtonStyle::Accent();
		bool disabled = false;
		std::function<void(GestureDetector::Event)> onClick{};
		Child child{};

		operator Child() const;
	};
}// namespace squi