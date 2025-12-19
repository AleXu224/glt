#pragma once

#include "core/animated.hpp"
#include "core/core.hpp"
#include "observer.hpp"


namespace squi {
	struct Modal : StatefulWidget {
		// Args
		Key key;
		Args widget{};
		VoidObservable closeEvent{};
		Child child;

		struct State : WidgetState<Modal> {
			VoidObserver closeObserver;
			Animated<Color> backgroundColor{
				.from = Color{0.f, 0.f, 0.f, 0.f},
				.duration = 200ms,
			};

			void observeCloseEvent();
			void initState() override;
			void widgetUpdated() override;

			Child build(const Element &) override;
		};
	};
}// namespace squi