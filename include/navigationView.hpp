#pragma once

#include "observer.hpp"
#include "widget.hpp"
#include <memory>

namespace squi {
	struct NavigationView {
		struct Page {
			std::string_view name;
			char32_t icon = 0xEA3A;
			Child content;
		};

		// Args
		Widget::Args widget{};
		bool expanded = true;
		std::function<void()> backAction{};
		std::vector<Page> pages{};

		struct Storage {
			// Data
			std::shared_ptr<Observable<Child>> contentChangeEvent = Observable<Child>::create();
			std::shared_ptr<Observable<Child>::Observer> contentChangeObserver;
		};

		operator Child() const;
	};
}// namespace squi