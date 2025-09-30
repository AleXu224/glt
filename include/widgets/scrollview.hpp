#pragma once

#include "core/core.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/scrollable.hpp"
#include "widgets/scrollbar.hpp"
#include "widgets/stack.hpp"
#include <GLFW/glfw3.h>


namespace squi {
	struct ScrollView : StatefulWidget {
		// Args
		Key key;
		Args widget;
		Args scrollWidget;
		Flex::Alignment alignment = Flex::Alignment::start;
		Axis direction = Axis::Vertical;
		float spacing = 0.f;
		Children children;

		struct State : WidgetState<ScrollView> {
			float scroll = 0.f;
			std::shared_ptr<ScrollViewData> controller{std::make_shared<ScrollViewData>()};
			Observable<float> scrollUpdater;
			Observer<float> scrollObserver;

			void initState() override {
				scrollObserver = scrollUpdater.observe([this](float newScroll) {
					auto clampedScroll = controller->clampScroll(newScroll);
					if (clampedScroll == scroll) return;
					setState([&]() {
						scroll = clampedScroll;
					});
				});
			}

			Child build(const Element &) override {
				return Gesture{
					.onUpdate = [this](const Gesture::State &state) {
						if (state.hovered) {
							auto scroll = state.getScroll();
							auto mainAxisScroll = widget->direction == Axis::Horizontal ? scroll.x : scroll.y;
							if (mainAxisScroll != 0.f) {
								scrollUpdater.notify(this->scroll - mainAxisScroll * 40.f);
							}
							// Allow scrolling horizontally when shift is held down
							if (state.inputState->isKeyDown(GLFW_KEY_LEFT_SHIFT) && widget->direction == Axis::Horizontal && scroll.y != 0.f) {
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
		};
	};
}// namespace squi