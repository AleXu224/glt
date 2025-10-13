#include "widgets/scrollview.hpp"

#include "widgets/gestureDetector.hpp"
#include "widgets/scrollable.hpp"
#include "widgets/scrollbar.hpp"
#include "widgets/stack.hpp"

namespace squi {
	void ScrollView::State::initState() {
		scrollObserver = scrollUpdater.observe([this](float newScroll) {
			auto clampedScroll = controller->clampScroll(newScroll);
			if (clampedScroll == scroll) return;
			setState([&]() {
				scroll = clampedScroll;
			});
		});
	}

	Child ScrollView::State::build(const Element &) {
		return Gesture{
			.onUpdate = [this](const Gesture::State &state) {
				if (state.hovered) {
					auto scroll = state.getScroll();
					auto mainAxisScroll = widget->direction == Axis::Horizontal ? scroll.x : scroll.y;
					if (mainAxisScroll != 0.f) {
						scrollUpdater.notify(this->scroll - mainAxisScroll * 40.f);
					}
					// Allow scrolling horizontally when shift is held down
					if (state.inputState->isKeyDown(GestureKey::leftShift) && widget->direction == Axis::Horizontal && scroll.y != 0.f) {
						scrollUpdater.notify(this->scroll - scroll.y * 40.f);
					}
				}
			},
			.child = Stack{
				.widget = widget->widget,
				.children{
					Scrollable{
						.widget = widget->scrollWidget,
						.alignment = widget->alignment,
						.direction = widget->direction,
						.spacing = widget->spacing,
						.scroll = scroll,
						.controller = controller,
						.children = widget->children,
					},
					Scrollbar{
						.direction = widget->direction,
						.controller = controller,
						.scrollUpdater = scrollUpdater,
						.scroll = scroll,
					},
				},
			},
		};
	}
}// namespace squi