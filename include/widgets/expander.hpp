#pragma once

#include "core/core.hpp"

namespace squi {
	struct Expander : StatefulWidget {
		// Args
		Key key;
		Child icon;
		std::string title;
		std::string subtitle;
		Child action;
		Child content;

		struct State : WidgetState<Expander> {
			bool expanded = false;

			Child build(const Element &element) override;
		};
	};
}// namespace squi