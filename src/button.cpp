#include "button.hpp"
#include "align.hpp"
#include "box.hpp"
#include "memory"
#include "text.hpp"

using namespace squi;

Button::operator squi::Child() const {
	auto storage = std::make_shared<Storage>(Storage{
        .color = style.color,
        .borderColor = style.borderColor,
    });
    auto newWidget = widget;
    newWidget.size.y = 32;
    newWidget.sizeBehavior.horizontal = SizeBehaviorType::MatchChild;
    newWidget.padding = style.padding;
    newWidget.onUpdate = [storage](Widget &widget) {
        Box::Impl &box = reinterpret_cast<Box::Impl &>(widget);
        auto &data = box.data();
        auto &gd = data.gestureDetector;

        if (gd.focused) box.setColor(storage->color * 0.8f);
        else if (gd.hovered) box.setColor(storage->color * 0.9f);
        else box.setColor(storage->color);
    };

	return Box{
		.widget{newWidget},
		.color{style.color},
		.borderColor{style.borderColor},
		.borderWidth = style.borderWidth,
		.borderRadius = style.borderRadius,
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