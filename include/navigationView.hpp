#pragma once

#include "observer.hpp"
#include "widget.hpp"

namespace squi {
	struct NavigationView {
		struct Page {
			std::string_view name;
			std::variant<int32_t, Child> icon = 0xef4a;
			Child content;
		};

		// Args
		Widget::Args widget{};
		bool expanded = true;
		std::function<void()> backAction{};
		std::vector<Page> pages{};

		struct Storage {
			// Data
			Observable<Child> contentChangeEvent{};
			Observer<Child> contentChangeObserver;
			Widget *currentPage = nullptr;
		};

		operator Child() const;
	};
}// namespace squi