#pragma once

#include "core/core.hpp"
#include "misc/scrollViewData.hpp"
#include "observer.hpp"

namespace squi {
	struct Scrollbar : StatefulWidget {
		// Args
		Key key;
		Axis direction;
		std::shared_ptr<ScrollViewData> controller{std::make_shared<ScrollViewData>()};
		Observable<float> scrollUpdater;
		float scroll;

		struct State : WidgetState<Scrollbar> {
			Child build(const Element &) override;
		};
	};
}// namespace squi