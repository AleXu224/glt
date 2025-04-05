#include "scrollbar.hpp"
#include "box.hpp"
#include "gestureDetector.hpp"
#include "wrapper.hpp"
#include <algorithm>
#include <memory>

using namespace squi;
using namespace std::chrono_literals;

Scrollbar::operator Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.controller = controller,
	});

	return Wrapper{
		.beforeLayout = [storage](Widget &widget, auto, auto) {
			if (storage->controller->contentMainAxis <= storage->controller->viewMainAxis)
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
					.widget = [&]() {
						switch (direction) {
							case Scrollable::Direction::vertical:
								return widget.withDefaultWidth(16.f).withDefaultPadding(3.f);
							case Scrollable::Direction::horizontal:
								return widget.withDefaultHeight(16.f).withDefaultPadding(3.f);
						}
					}(),
					.color{0},
					.child{
						GestureDetector{
							.onPress = [storage](auto) {
								storage->scrollDragStart = storage->scroll;
							},
							.onUpdate = [storage, direction = direction](GestureDetector::Event event) {
								const float contentMainAxisSize = [&]() {
									switch (direction) {
										case Scrollable::Direction::vertical:
											return event.widget.state.parent->getContentRect().height() - event.widget.getSize().y;
										case Scrollable::Direction::horizontal:
											return event.widget.state.parent->getContentRect().width() - event.widget.getSize().x;
									}
								}();
								if (event.state.focused && *event.widget.flags.visible) {
									switch (direction) {
										case Scrollable::Direction::vertical:
											storage->scroll = storage->scrollDragStart + event.state.getDragOffset().y / contentMainAxisSize;
											break;
										case Scrollable::Direction::horizontal:
											storage->scroll = storage->scrollDragStart + event.state.getDragOffset().x / contentMainAxisSize;
											break;
									}
									storage->scroll = (std::clamp) (storage->scroll, 0.f, 1.0f);
									storage->controller->scroll = storage->scroll * (storage->controller->contentMainAxis - storage->controller->viewMainAxis);
									storage->controller->onScrollChange.notify(storage->controller->scroll);
								}

								const auto currentTime = std::chrono::steady_clock::now();
								if (currentTime - storage->lastHoverTime > 1s) {
									switch (direction) {
										case Scrollable::Direction::vertical:
											event.widget.state.width = 2.0f;
											event.widget.state.margin = event.widget.state.margin->withLeft(8.0f);
											break;
										case Scrollable::Direction::horizontal:
											event.widget.state.height = 2.0f;
											event.widget.state.margin = event.widget.state.margin->withTop(8.0f);
											break;
									}
								} else {
									switch (direction) {
										case Scrollable::Direction::vertical:
											event.widget.state.width = 10.f;
											event.widget.state.margin = event.widget.state.margin->withLeft(0);
											break;
										case Scrollable::Direction::horizontal:
											event.widget.state.height = 10.f;
											event.widget.state.margin = event.widget.state.margin->withTop(0);
											break;
									}
								}
							},
							.child{
								Box{
									.widget{
										.onInit = [direction = direction](Widget &w) {
											switch (direction) {
												case Scrollable::Direction::vertical:
													w.state.width = 2.f;
													w.state.margin = Margin{}.withLeft(8.f);
													w.state.sizeConstraints.minHeight = 24.f;
													break;
												case Scrollable::Direction::horizontal:
													w.state.height = 2.f;
													w.state.margin = Margin{}.withTop(8.f);
													w.state.sizeConstraints.minWidth = 24.f;
													break;
											}
										},
										.onLayout = [storage, direction = direction](Widget &, vec2 &maxSize, vec2 &minSize) {
											if (storage->controller->contentMainAxis == 0.f) return;
											switch (direction) {
												case Scrollable::Direction::vertical:
													maxSize.y *= (storage->controller->viewMainAxis / storage->controller->contentMainAxis);
													minSize.y = std::min(minSize.y, maxSize.y);
													break;
												case Scrollable::Direction::horizontal:
													maxSize.x *= (storage->controller->viewMainAxis / storage->controller->contentMainAxis);
													minSize.x = std::min(minSize.x, maxSize.x);
													break;
											}
										},
										.onArrange = [storage, direction = direction](Widget &widget, vec2 &pos) {
											if (!*widget.state.parent) return;

											auto &controller = *storage->controller;
											if (controller.contentMainAxis <= controller.viewMainAxis)
												storage->scroll = 0;
											else
												storage->scroll = controller.scroll / (controller.contentMainAxis - controller.viewMainAxis);
											const auto maxOffset = widget.state.parent->getContentSize() - widget.getLayoutSize();

											switch (direction) {
												case Scrollable::Direction::vertical:
													pos.y += maxOffset.y * storage->scroll;
													break;
												case Scrollable::Direction::horizontal:
													pos.x += maxOffset.x * storage->scroll;
													break;
											}
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