#pragma once

#include "core/animated.hpp"
#include "core/core.hpp"
#include "core/direction.hpp"

namespace squi {
	struct SlideIn : StatefulWidget {
		// Args
		Key key;
		Args widget;
		Direction direction = Direction::top;
		Child child;

		struct State : WidgetState<SlideIn> {
			Animated<float> offsetProgress{.from = 1.f};

			void initState() override {
				offsetProgress.mount(this);
				offsetProgress = 0.f;
			}

			Child build(const Element &) override;
		};
	};
}// namespace squi