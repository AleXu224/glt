#pragma once

#include "core/animated.hpp"
#include "core/core.hpp"

namespace squi {
	struct SideNav : StatefulWidget {
		struct Page {
			std::string name;
			std::variant<int32_t, Child> icon = 0xef4a;
			Child content;
		};

		// Args
		Key key;
		Args widget;
		std::function<void()> backAction;
		bool defaultExpanded = true;
		std::vector<Page> pages{};

		struct State : WidgetState<SideNav> {
			int64_t currentPageIndex{0};
			Animated<float> navWidth{.from = 320.f};
			bool expanded = true;

			void initState() override {
				expanded = widget->defaultExpanded;
				navWidth.from = expanded ? 320.f : 48.f;
				navWidth.to = expanded ? 320.f : 48.f;
				navWidth.mount(this);
			}

			Child build(const Element &element) override;
		};
	};
}// namespace squi