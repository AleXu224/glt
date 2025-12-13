#pragma once

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

			void observeCloseEvent();
			void initState() override;
			void widgetUpdated() override;

			Child build(const Element &) override;
		};
	};
}// namespace squi