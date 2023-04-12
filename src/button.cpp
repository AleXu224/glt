#include "button.hpp"
#include "align.hpp"
#include "box.hpp"
#include "memory"
#include "text.hpp"

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
    auto newWidget = widget;
    newWidget.sizeBehavior.horizontal = SizeBehaviorType::MatchChild;
    newWidget.sizeBehavior.vertical = SizeBehaviorType::MatchChild;
    newWidget.padding = style.padding;
	newWidget.onUpdate = [storage](Widget &widget) {
		auto &box = reinterpret_cast<Box::Impl &>(widget);
		auto &data = box.data();
        auto &gd = data.gestureDetector;

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
		}
	};

	return Box{
		.widget{newWidget},
		.color{style.color},
		.borderColor{style.borderColor},
		.borderWidth = style.borderWidth,
		.borderRadius = style.borderRadius,
		.borderPosition = style.borderPosition,
		.child = Align{
			.child{
				Text{
					.text{std::string(text)},
					.color{style.textColor},
				},
			},
		},
	};
}