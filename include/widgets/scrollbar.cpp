#include "widgets/scrollbar.hpp"

#include "layoutBuilder.hpp"
#include "widgets/box.hpp"
#include "widgets/gestureDetector.hpp"

namespace squi {
	Child Scrollbar::State::build(const Element &) {
		return Box{
			.widget{
				.width = widget->direction == Axis::Horizontal ? SizeVariant(Size::Expand) : SizeVariant(16.f),
				.height = widget->direction == Axis::Vertical ? SizeVariant(Size::Expand) : SizeVariant(16.f),
			},
			.color = 0xFFFFFF0F,
			.child = LayoutBuilder{
				.builder = [this](BoxConstraints constraints) {
					BoxConstraints sizeConstraints{};
					float pScrollSize = 1.f;
					if (widget->controller->contentMainAxis > 0.f) {
						pScrollSize = widget->controller->viewMainAxis / widget->controller->contentMainAxis;
					}
					auto mainAxisDragMultiplier = 0.f;
					if (widget->direction == Axis::Horizontal) {
						sizeConstraints.maxWidth = constraints.maxWidth * pScrollSize;
						if (constraints.maxWidth > 0.f)
							mainAxisDragMultiplier = sizeConstraints.maxWidth / constraints.maxWidth;
					} else {
						sizeConstraints.maxHeight = constraints.maxHeight * pScrollSize;
						if (constraints.maxHeight > 0.f)
							mainAxisDragMultiplier = sizeConstraints.maxHeight / constraints.maxHeight;
					}

					return Gesture{
						.onDrag = [this, mainAxisDragMultiplier](const Gesture::State &state) {
							auto deltaDrag = state.getDragDelta();
							auto delta = widget->direction == Axis::Horizontal ? deltaDrag.x : deltaDrag.y;

							if (delta != 0.f)
								widget->scrollUpdater.notify(widget->scroll + delta * mainAxisDragMultiplier);
						},
						.child = Box{
							.widget{
								.sizeConstraints = sizeConstraints,
							},
							.color = 0xFFFFFF8B,
						},
					};
				},
			},
		};
	}
}// namespace squi