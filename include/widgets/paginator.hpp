#pragma once

#include "core/core.hpp"

namespace squi {
	struct Paginator : StatefulWidget {
		// Args
		Key key;
		Args widget;
		uint32_t itemsPerPage = 50;
		std::function<uint32_t()> getItemCount;
		std::function<Child(uint32_t offset, uint32_t count)> builder;

		struct State : WidgetState<Paginator> {
			size_t current_page = 0;

			Child build(const Element &) override;
		};
	};
}// namespace squi