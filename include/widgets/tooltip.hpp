#pragma once

#include "core/core.hpp"

namespace squi {
	struct Tooltip : StatefulWidget {
		// Args
		Key key;
		std::string text;
		Child child;

		struct State : WidgetState<Tooltip> {
			~State();

			Child build(const Element &element) override;
		};
	};

	struct TooltipWithTarget : StatefulWidget {
		// Args
		Key key;
		Key targetKey;
		std::string text;
		Child child;

		struct State : WidgetState<TooltipWithTarget> {
			~State();

			Child build(const Element &element) override;
		};
	};
}// namespace squi