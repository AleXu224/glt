#include "widgets/scrollbar.hpp"

#include "animatedBox.hpp"
#include "layoutBuilder.hpp"
#include "widgets/box.hpp"
#include "widgets/gestureDetector.hpp"

namespace squi {
	[[nodiscard]] Padding Scrollbar::State::getPadding() const {
		auto ret = Padding{3.f};

		if (hovered || focused) return ret;

		if (widget->direction == Axis::Horizontal) {
			ret.top = 3.f + 4.f;
		} else {
			ret.left = 3.f + 4.f;
		}

		return ret;
	}

	Child Scrollbar::State::build(const Element &) {
		// Wrap the whole thing in a layout builder so that we can get the sizing of the scrollable
		return LayoutBuilder{
			.widget{
				.alignment = Alignment::BottomRight,
			},
			.builder = [this](BoxConstraints) -> Child {
				if (widget->controller->contentMainAxis <= widget->controller->viewMainAxis)
					return nullptr;

				return Gesture{
					.onEnter = [this](auto &&) {
						setState([this]() {
							hovered = true;
						});
					},
					.onLeave = [this](auto &&) {
						setState([this]() {
							hovered = false;
						});
					},
					.child = AnimatedBox{
						.widget{
							.width = widget->direction == Axis::Horizontal ? SizeVariant(Size::Expand) : SizeVariant(12.f),
							.height = widget->direction == Axis::Vertical ? SizeVariant(Size::Expand) : SizeVariant(12.f),
							.padding = getPadding(),
						},
						.color = hovered || focused ? 0xFFFFFF0F : Color::transparent,
						.child = LayoutBuilder{
							.widget{
								.width = Size::Expand,
								.height = Size::Expand,
							},
							.builder = [this](BoxConstraints constraints) {
								BoxConstraints sizeConstraints{};
								float pScrollSize = 1.f;
								if (widget->controller->contentMainAxis > 0.f) {
									pScrollSize = widget->controller->viewMainAxis / widget->controller->contentMainAxis;
								}
								auto mainAxisDragMultiplier = 0.f;
								auto alignment = Alignment::TopLeft;
								if (widget->direction == Axis::Horizontal) {
									sizeConstraints.maxWidth = constraints.maxWidth * pScrollSize;
									if (constraints.maxWidth > 0.f)
										mainAxisDragMultiplier = widget->controller->contentMainAxis / constraints.maxWidth;

									if (widget->controller->contentMainAxis != widget->controller->viewMainAxis)
										alignment.horizontal = widget->scroll / (widget->controller->contentMainAxis - widget->controller->viewMainAxis);
								} else {
									sizeConstraints.maxHeight = constraints.maxHeight * pScrollSize;
									if (constraints.maxHeight > 0.f)
										mainAxisDragMultiplier = widget->controller->contentMainAxis / constraints.maxHeight;

									if (widget->controller->contentMainAxis != widget->controller->viewMainAxis)
										alignment.vertical = widget->scroll / (widget->controller->contentMainAxis - widget->controller->viewMainAxis);
								}

								return Gesture{
									.widget{
										.alignment = alignment,
									},
									.onFocus = [this](const Gesture::State &) {
										dragStartScroll = widget->scroll;
										setState([this]() {
											focused = true;
										});
									},
									.onFocusLoss = [this](const Gesture::State &) {
										setState([this]() {
											focused = false;
										});
									},
									.onDrag = [this, mainAxisDragMultiplier](const Gesture::State &state) {
										auto deltaDrag = state.getDragOffset();
										auto delta = widget->direction == Axis::Horizontal ? deltaDrag.x : deltaDrag.y;

										if (delta != 0.f)
											widget->scrollUpdater.notify(dragStartScroll + delta * mainAxisDragMultiplier);
									},
									.child = Box{
										.widget{
											.sizeConstraints = sizeConstraints,
										},
										.color = 0xFFFFFF8B,
										.borderRadius = 3.f,
									},
								};
							},
						},
					},
				};
			},
		};
	}
}// namespace squi