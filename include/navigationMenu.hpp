#pragma once

#include "observer.hpp"
#include "widget.hpp"
#include <functional>
#include <vector>

namespace squi {
	struct NavigationMenu {
		struct Item {
			std::string_view name;
			char32_t icon = 0xEA3A;
			std::function<void()> onClick;
		};

		// Args
		Widget::Args widget;
		std::function<void()> backAction{};
		std::vector<Item> items{};

		struct Storage {
			// Data
			bool isExpanded = true;
			std::shared_ptr<Observable<bool>> isExpandedEvent = Observable<bool>::create();
			std::shared_ptr<Observable<bool>::Observer> expandedObserver;
			std::shared_ptr<VoidObservable> selectionEvent = VoidObservable::create();
		};

		operator Child() const;
	};
}// namespace squi