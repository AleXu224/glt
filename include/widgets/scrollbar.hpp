#pragma once

#include "core/core.hpp"
#include "misc/scrollViewData.hpp"
#include "observer.hpp"

namespace squi {
	struct Scrollbar : StatefulWidget {
		// Args
		Key key;
		Axis direction = Axis::Vertical;
		std::shared_ptr<ScrollViewData> controller{std::make_shared<ScrollViewData>()};
		Observable<float> scrollUpdater;
		float scroll;

		struct State : WidgetState<Scrollbar> {
			float dragStartScroll = 0.f;
			bool hovered = false;
			bool focused = false;

			[[nodiscard]] Padding getPadding() const;

			Child build(const Element &) override;
		};
	};
}// namespace squi