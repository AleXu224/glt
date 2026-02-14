#pragma once

#include "core/core.hpp"

namespace squi {
	// The reason for this widget is that sometimes you need a more local context
	struct Builder : StatelessWidget {
		// Args
		Key key;
		std::function<Child(const Element &)> builder;

		[[nodiscard]] Child build(const Element &element) const {
			return builder(element);
		}
	};
}// namespace squi