#include "button.hpp"
#include "box.hpp"
#include "gestureDetector.hpp"
#include "memory"
#include <debugapi.h>

using namespace squi;

Button::operator squi::Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.color = style.color,
		.colorHover = style.colorHover,
		.colorActive = style.colorActive,
		.colorDisabled = style.colorDisabled,
		.borderColor = style.borderColor,
		.borderColorHover = style.borderColorHover,
		.borderColorActive = style.borderColorActive,
		.borderColorDisabled = style.borderColorDisabled,
		.disabled = disabled,
	});

	return GestureDetector{
		.onClick = [storage, onClick = onClick](GestureDetector::Event event) {
			if (!storage->disabled && onClick) {
				onClick(event);
			}
		},
		.onUpdate = [storage](GestureDetector::Event event) {
			auto &box = reinterpret_cast<Box::Impl &>(event.widget);

			if (storage->disabled) {
				box.setColor(storage->colorDisabled);
				box.setBorderColor(storage->borderColorDisabled);
			} else if (event.state.focused) {
				box.setColor(storage->colorActive);
				box.setBorderColor(storage->borderColorActive);
			} else if (event.state.hovered) {
				box.setColor(storage->colorHover);
				box.setBorderColor(storage->borderColorHover);
			} else {
				box.setColor(storage->color);
				box.setBorderColor(storage->borderColor);
			}
		},
		.child{
			Box{
				.widget = widget.withDefaultWidth(Size::Shrink).withDefaultHeight(Size::Shrink).withDefaultPadding(Padding{6, 12, 6, 12}).withSizeConstraints(widget.sizeConstraints.withDefaultMinWidth(32.f).withDefaultMinHeight(32.f)),
				.color{style.color},
				.borderColor{style.borderColor},
				.borderWidth = style.borderWidth,
				.borderRadius = style.borderRadius,
				.borderPosition = style.borderPosition,
				.child = child,
			},
		},
	};
}