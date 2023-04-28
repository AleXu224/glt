#include "button.hpp"
#include "align.hpp"
#include "box.hpp"
#include "gestureDetector.hpp"
#include "memory"
#include "text.hpp"
#include <debugapi.h>
#include <iterator>

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
		.onClick = [storage, onClick = onClick](Widget &w, auto &gd) {
			if (!storage->disabled && onClick) {
				onClick();
			} },
		.onUpdate = [storage](Widget &widget, GestureDetector::Storage &gd) {
			auto &box = reinterpret_cast<Box::Impl &>(widget);

			if (storage->disabled) {
				box.setColor(storage->colorDisabled);
				box.setBorderColor(storage->borderColorDisabled);
			} else if (gd.focused) {
				box.setColor(storage->colorActive);
				box.setBorderColor(storage->borderColorActive);
			} else if (gd.hovered) {
				box.setColor(storage->colorHover);
				box.setBorderColor(storage->borderColorHover);
			} else {
				box.setColor(storage->color);
				box.setBorderColor(storage->borderColor);
			} },
		.child{
			Box{
				.widget{
					.width = width,
					.height = height,
					.sizeConstraints = sizeConstraints,
					.margin = margin,
					.padding = padding,
				},
				.color{style.color},
				.borderColor{style.borderColor},
				.borderWidth = style.borderWidth,
				.borderRadius = style.borderRadius,
				.borderPosition = style.borderPosition,
				.child = Align{
					.child{
						Text{
							.text{std::string(text)},
							// TODO: Make this work
							.lineWrap = true,
							.color{style.textColor},
						},
					},
				},
			},
		},
	};
}