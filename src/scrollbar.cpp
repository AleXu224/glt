#include "gestureDetector.hpp"
#define NOMINMAX
#include "scrollbar.hpp"
#include <algorithm>
#include <memory>

using namespace squi;
using namespace std::chrono_literals;

Scrollbar::operator Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.controller = controller,
	});
	auto newWidget = widget;
	// newWidget.size.x = 16;
	// newWidget.sizeBehavior.horizontal = SizeBehaviorType::None;
	newWidget.width = 16.0f;
	newWidget.padding = 3;
	newWidget.onUpdate = [storage](Widget &widget) {
		auto &gd = std::any_cast<GestureDetector::Storage &>(widget.state.properties.at("gestureDetector"));
		const auto currentTime = std::chrono::steady_clock::now();
		if (gd.hovered || gd.focused) {
			storage->lastHoverTime = std::chrono::steady_clock::now();
		}

		using namespace std::chrono_literals;
		auto &w = reinterpret_cast<Box::Impl &>(widget);
		if (currentTime - storage->lastHoverTime > 1s) {
			w.setColor(Color::HEX(0));
		} else {
			w.setColor(Color::HEX(0xFFFFFF0F));
		}

		const auto &controller = *storage->controller;
		widget.flags.visible = controller.contentHeight > controller.viewHeight;
	};

	return GestureDetector{
		.child{
			Box{
				.widget{std::move(newWidget)},
				.color{Color::HEX(0xFFFFFF0F)},
				.child{
					GestureDetector{
						.onPress = [storage](auto &w, auto &gd) { storage->scrollDragStart = storage->scroll; },
						.child{
							Box{
								.widget{
									.width = 10.0f,
									.height = 32.f,
									.onUpdate = [storage](Widget &widget) {
										auto &gestureDetector = std::any_cast<GestureDetector::Storage &>(widget.state.properties.at("gestureDetector"));

										const float contentHeight = widget.state.parent->getContentRect().height() - widget.getSize().y;
										if (gestureDetector.focused && widget.flags.visible) {
											storage->scroll = storage->scrollDragStart + gestureDetector.getDragOffset().y / contentHeight;
											storage->scroll = (std::clamp)(storage->scroll, 0.f, 1.0f);
											storage->controller->scroll = storage->scroll * (storage->controller->contentHeight - storage->controller->viewHeight);
										}

										const auto currentTime = std::chrono::steady_clock::now();
										if (currentTime - storage->lastHoverTime > 1s) {
											widget.state.sizeMode.width = 2.0f;
											widget.state.margin.left = 8;
										} else {
											widget.state.sizeMode.width = 10.0f;
											widget.state.margin.left = 0;
										} 
									},
									.onLayout = [storage](Widget &widget, vec2 &maxSize, vec2 &minSize) {
										if (!widget.state.parent) return;

										auto &controller = *storage->controller;

										if (controller.contentHeight <= controller.viewHeight) storage->scroll = 0;
										else storage->scroll = controller.scroll / (controller.contentHeight - controller.viewHeight);
										minSize.y = controller.viewHeight / controller.contentHeight * maxSize.y;
										minSize.y = std::max(minSize.y, 24.0f);
									},
									.onArrange = [storage](Widget &widget, vec2 &pos) {
										if (!widget.state.parent) return;

										const auto maxOffset = widget.state.parent->getContentSize() - widget.getLayoutSize();

										pos.y += maxOffset.y * storage->scroll; },
								},
								.color{Color::HEX(0xFFFFFF8B)},
								.borderRadius = 2,
							},
						},
					},
				},
			},
		},
	};
}