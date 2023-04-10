#include "scrollbar.hpp"

using namespace squi;

Scrollbar::operator Child() const {
	auto storage = std::make_shared<Storage>();
	auto newWidget = widget;
	newWidget.size.x = 12;
	newWidget.sizeBehavior.horizontal = SizeBehaviorType::None;
	newWidget.padding = 3;

	return Child(Box{
		.widget{std::move(newWidget)},
		.color{Color::HEX(0xFFFFFF0F)},
		.borderRadius = 999,
		.child{
			Box{
				.widget{
					.size{6},
                    .onInit = [storage](Widget &widget) {
                        auto &data = widget.data();

						data.gestureDetector.onPress = [storage](auto &gd) {
							storage->scrollDragStart = storage->scroll;
						};
                    },
					.onUpdate = [storage, onScroll = onScroll](Widget &widget) {
						auto &data = widget.data();
						auto &gestureDetector = data.gestureDetector;
                        const float contentHeight = data.parent->getContentRect().height() - data.size.y;
						if (gestureDetector.focused) {
							storage->scroll = storage->scrollDragStart + gestureDetector.getDragOffset().y / contentHeight;
                            storage->scroll = (std::clamp)(storage->scroll, 0.f, 1.0f);
							if (onScroll) onScroll(widget);
						}
                        data.margin.top = storage->scroll * contentHeight;
					},
				},
				.color{Color::HEX(0xFFFFFF8B)},
				.borderRadius = 999,
			},
		},
	});
}