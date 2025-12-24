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
}// namespace squi