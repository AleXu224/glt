#pragma once

#include "core/core.hpp"

namespace squi {
	struct Tooltip : StatelessWidget {
		// Args
		Key key;
		std::string text;
		Child child;

		[[nodiscard]] Child build(const Element &) const;
	};

	struct TooltipWithTarget : StatefulWidget {
		// Args
		Key key;
		Key targetKey;
		std::string text;
		Child child;

		struct State : WidgetState<TooltipWithTarget> {
			Child build(const Element &element) override;
		};
	};
}// namespace squi