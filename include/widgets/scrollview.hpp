#pragma once

#include "core/core.hpp"
#include "observer.hpp"
#include "widgets/flex.hpp"
#include "widgets/misc/scrollViewData.hpp"


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

			void initState() override;

			Child build(const Element &) override;
		};
	};
}// namespace squi