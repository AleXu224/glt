#pragma once

#include "button.hpp"
#include "contextMenu.hpp"
#include "widget.hpp"


namespace squi {
	struct DropdownButton {
		// Args
		squi::Widget::Args widget{};
		ButtonStyle style{};
		std::string_view text{};
		std::vector<ContextMenu::Item> items{};

		Observable<const std::string &> textUpdater{};
		Observable<std::vector<ContextMenu::Item>> itemsUpdater{};

		struct Storage {
			// Data
			std::vector<ContextMenu::Item> items;
		};

		operator squi::Child() const;
	};
}// namespace squi