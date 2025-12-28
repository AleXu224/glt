#pragma once

#include "core/core.hpp"
#include "widgets/flex.hpp"

namespace squi {
	struct Column : StatelessWidget {
		using Alignment = Flex::Alignment;
		using JustifyContent = Flex::JustifyContent;
		// Args
		Key key{};
		Args widget{};
		Alignment crossAxisAlignment = Alignment::start;
		JustifyContent justifyContent = JustifyContent::start;
		float spacing = 0.f;
		Children children{};

		[[nodiscard]] Child build(const Element &) const {
			return Flex{
				.widget = widget,
				.direction = Axis::Vertical,
				.crossAxisAlignment = crossAxisAlignment,
				.justifyContent = justifyContent,
				.spacing = spacing,
				.children = children,
			};
		}
	};
}// namespace squi