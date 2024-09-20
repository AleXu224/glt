#include "button.hpp"
#include "align.hpp"
#include "box.hpp"
#include "gestureDetector.hpp"
#include "memory"
#include "text.hpp"

using namespace squi;

Button::operator squi::Child() const {
	Child ret = GestureDetector{
		.onClick = [onClick = onClick](GestureDetector::Event event) {
			auto &disabled = event.widget.customState.get<bool>("disabled");
			if (!disabled && onClick) {
				onClick(event);
			}
		},
		.onUpdate = [](GestureDetector::Event event) {
			auto &box = reinterpret_cast<Box::Impl &>(event.widget);
			auto &disabled = event.widget.customState.get<bool>("disabled");
			auto &style = event.widget.customState.get<ButtonStyle>("style");

			if (disabled) {
				box.setColor(style.colorDisabled);
				box.setBorderColor(style.borderColorDisabled);
			} else if (event.state.focused) {
				box.setColor(style.colorActive);
				box.setBorderColor(style.borderColorActive);
			} else if (event.state.hovered) {
				box.setColor(style.colorHover);
				box.setBorderColor(style.borderColorHover);
			} else {
				box.setColor(style.color);
				box.setBorderColor(style.borderColor);
			}
		},
		.child{
			Box{
				.widget = widget.withDefaultWidth(Size::Shrink).withDefaultHeight(Size::Shrink).withDefaultPadding(Padding{6, 12, 6, 12}).withSizeConstraints(widget.sizeConstraints.withDefaultMinWidth(32.f).withDefaultMinHeight(32.f)),
				.color{style.color},
				.borderColor{style.borderColor},
				.borderWidth{style.borderWidth},
				.borderRadius{style.borderRadius},
				.borderPosition = style.borderPosition,
				.child = child ? child : Align{.child = Text{.text = text}},
			},
		},
	};

	ret->customState.add("style", style);
	ret->customState.add("disabled", disabled);

	return ret;
}