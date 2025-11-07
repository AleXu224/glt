#pragma once

#include "core/core.hpp"

namespace squi {
	struct Card : StatelessWidget {
		// Args
		Key key;
		Args widget;
		Child child;

		[[nodiscard]] Child build(const Element &) const;
	};
}// namespace squi