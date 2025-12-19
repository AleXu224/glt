#include "slideIn.hpp"
#include "widgets/container.hpp"
#include "widgets/offset.hpp"

namespace squi {
	core::Child SlideIn::State::build(const Element &) {
		auto newWidget = widget->widget;
		newWidget.width = newWidget.width.value_or(Size::Wrap);
		newWidget.height = newWidget.height.value_or(Size::Wrap);
		if (!widget->followChild) {
			newWidget.alignment = newWidget.alignment.value_or(Alignment::TopLeft);
		}

		return Container{
			.widget = newWidget,
			.child = Offset{
				.calculateContentBounds = [this](const Rect &bounds, const SingleChildRenderObject &) {
					vec2 offset{};
					switch (widget->direction) {
						case Direction::top:
							offset.y = -bounds.height() * offsetProgress.getValue();
							break;
						case Direction::bottom:
							offset.y = bounds.height() * offsetProgress.getValue();
							break;
						case Direction::left:
							offset.x = -bounds.width() * offsetProgress.getValue();
							break;
						case Direction::right:
							offset.x = bounds.width() * offsetProgress.getValue();
							break;
					}
					return Rect::fromPosSize(bounds.getTopLeft() + offset, bounds.size());
				},
				.child = widget->child,
			},
		};
	}
}// namespace squi