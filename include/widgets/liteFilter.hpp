#pragma once

#include "core/core.hpp"
#include <set>

namespace squi {
	struct LiteFilter : StatefulWidget {
		struct Item {
			std::string name;
			std::function<void(bool)> onUpdate{};
		};

		// Args
		Key key;
		std::vector<Item> items;

		struct State : WidgetState<LiteFilter> {
			std::set<size_t> selectedIndices;

			[[nodiscard]] Children createButtons();

			void initState() override {
				for (size_t i = 0; i < widget->items.size(); ++i) {
					selectedIndices.insert(i);
				}
			}

			Child build(const Element &element) override;
		};
	};
}// namespace squi