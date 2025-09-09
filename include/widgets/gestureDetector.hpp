#pragma once

#include "core/core.hpp"


namespace squi {
	struct GestureDetector : StatelessWidget {
		// Args
		Child child{};

		Child build(const Element &) const {
			return child;
		}
	};
}// namespace squi