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
		bool followChild = false;
		bool visible = true;
		std::chrono::milliseconds duration = 200ms;
		std::function<float(float)> curve = core::Curve::easeOutCubic;
		std::function<void()> onFinish{};
		std::function<void()> onDismiss{};
		Child child;

		struct State : WidgetState<SlideIn> {
			Animated<float> offsetProgress{.from = 1.f};
			bool exiting = false;

			void initState() override;
			void widgetUpdated() override;

			Child build(const Element &) override;
		};
	};
}// namespace squi