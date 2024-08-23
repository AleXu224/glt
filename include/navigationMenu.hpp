#pragma once

#include "observer.hpp"
#include "widget.hpp"
#include <functional>
#include <vector>

namespace squi {
	struct NavigationMenu {
		struct Item {
			std::string_view name;
			std::variant<int32_t, Child> icon = 0xEA3A;
			std::function<void()> onClick;
		};

		// Args
		Widget::Args widget{};
		bool expanded = true;
		std::function<void()> backAction{};
		std::vector<Item> items{};

		struct Storage {
			// Data
			bool isExpanded = true;
			Observable<bool> isExpandedEvent{};
			Observer<bool> expandedObserver;
			VoidObservable selectionEvent{};
		};

		operator Child() const;
	};
}// namespace squi