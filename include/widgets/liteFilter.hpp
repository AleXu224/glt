#pragma once

#include "observer.hpp"
#include "widget.hpp"

namespace squi {
	struct LiteFilter {
        struct Item{
            std::string name;
            std::function<void(bool)> onUpdate{};
        };
        
		// Args
		squi::Widget::Args widget{};
        bool multiSelect = false;
        std::vector<Item> items;

		struct Storage {
			// Data
            std::vector<Item> items;
            bool allSelected = true;
            uint32_t selectedItemsCount = 0;
            std::vector<Observable<bool>> events{};
		};

		operator squi::Child() const;
	};
}// namespace squi