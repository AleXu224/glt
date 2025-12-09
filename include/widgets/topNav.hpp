#pragma once

#include "core/core.hpp"

namespace squi {
	struct TopNav : StatefulWidget {
        struct Page{
            std::string name;
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