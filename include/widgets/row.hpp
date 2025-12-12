#pragma once

#include "core/core.hpp"
#include "widgets/flex.hpp"

namespace squi {
	struct Row : StatelessWidget {
		using Alignment = Flex::Alignment;
		// Args
		Key key{};
		Args widget{};
		Alignment crossAxisAlignment = Alignment::start;
		float spacing = 0.f;
		Children children{};

		[[nodiscard]] Child build(const Element &) const {
			return Flex{
				.widget = widget,
				.direction = Axis::Horizontal,
				.crossAxisAlignment = crossAxisAlignment,
				.spacing = spacing,
				.children = children,
			};
		}
	};
}// namespace squi