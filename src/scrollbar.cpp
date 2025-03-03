#include "scrollbar.hpp"
#include "box.hpp"
#include "gestureDetector.hpp"
#include "registerEvent.hpp"
#include <algorithm>
#include <memory>

using namespace squi;
using namespace std::chrono_literals;

Scrollbar::operator Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.controller = controller,
	});

	return RegisterEvent{
		.beforeLayout = [storage](Widget &widget, auto, auto) {
			if (storage->controller->contentHeight <= storage->controller->viewHeight)
				widget.flags.visible = false;
			else
				widget.flags.visible = true;
		},
		.child = GestureDetector{
			.onUpdate = [storage](GestureDetector::Event event) {
				if (!*event.widget.flags.visible) return;
				const auto currentTime = std::chrono::steady_clock::now();
				if (event.state.hovered || event.state.focused) {
					storage->lastHoverTime = std::chrono::steady_clock::now();
				}

				using namespace std::chrono_literals;
				auto &w = reinterpret_cast<Box::Impl &>(event.widget);
				if (currentTime - storage->lastHoverTime > 1s) {
					w.setColor(0);
				} else {
					w.setColor(0xFFFFFF0F);
				}
			},
			.child{
				Box{
					.widget = widget.withDefaultWidth(16.f).withDefaultPadding(3.f),
					.color{0},
					.child{
						GestureDetector{
							.onPress = [storage](auto) {
								storage->scrollDragStart = storage->scroll;
							},
							.onUpdate = [storage](GestureDetector::Event event) {
								const float contentHeight = event.widget.state.parent->getContentRect().height() - event.widget.getSize().y;
								if (event.state.focused && *event.widget.flags.visible) {
									storage->scroll = storage->scrollDragStart + event.state.getDragOffset().y / contentHeight;
									storage->scroll = (std::clamp)(storage->scroll, 0.f, 1.0f);
									storage->controller->scroll = storage->scroll * (storage->controller->contentHeight - storage->controller->viewHeight);
									storage->controller->onScrollChange.notify(storage->controller->scroll);
								}

								const auto currentTime = std::chrono::steady_clock::now();
								if (currentTime - storage->lastHoverTime > 1s) {
									event.widget.state.width = 2.0f;
									event.widget.state.margin = event.widget.state.margin->withLeft(8.0f);
								} else {
									event.widget.state.width = 10.f;
									event.widget.state.margin = event.widget.state.margin->withLeft(0);
								}
							},
							.child{
								Box{
									.widget{
										.width = 2.0f,
										.height = Size::Expand,
										.sizeConstraints{
											.minHeight = 24.f,
										},
										.margin = Margin{}.withLeft(8.f),
										.onLayout = [storage](Widget &, vec2 &maxSize, vec2 &minSize) {
											if (storage->controller->contentHeight == 0.f) return;
											maxSize.y *= (storage->controller->viewHeight / storage->controller->contentHeight);
											minSize.y = std::min(minSize.y, maxSize.y);
										},
										.onArrange = [storage](Widget &widget, vec2 &pos) {
											if (!*widget.state.parent) return;

											auto &controller = *storage->controller;
											if (controller.contentHeight <= controller.viewHeight)
												storage->scroll = 0;
											else
												storage->scroll = controller.scroll / (controller.contentHeight - controller.viewHeight);
											const auto maxOffset = widget.state.parent->getContentSize() - widget.getLayoutSize();

											pos.y += maxOffset.y * storage->scroll;
										},
									},
									.color{0xFFFFFF8B},
									.borderRadius{2},
								},
							},
						},
					},
				},
			},
		},
	};
}