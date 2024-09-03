#pragma once

#include "widget.hpp"
#include "button.hpp"
#include "contextMenu.hpp"

namespace squi {
	struct DropdownButton {
		// Args
		squi::Widget::Args widget{};
        ButtonStyle style{};
        std::string_view text{};
        std::vector<ContextMenu::Item> items{};

		struct Storage {
			// Data
		};

		operator squi::Child() const;
	};
}// namespace squi