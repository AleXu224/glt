#pragma once

#include "core/core.hpp"

namespace squi {
	struct TopNav : StatefulWidget {
		struct Page {
			std::string name;
			std::variant<int32_t, Child, std::monostate> icon = std::monostate{};
			Child content;
		};

		// Args
		Key key;
		Args widget;
		std::vector<Page> pages{};

		struct State : WidgetState<TopNav> {
			int64_t currentPageIndex{0};

			Children getButtons();

			Child build(const Element &element) override;
		};
	};
}// namespace squi