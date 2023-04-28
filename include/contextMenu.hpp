#ifndef SQUI_CONTEXTMENU_HPP
#define SQUI_CONTEXTMENU_HPP

#include "widget.hpp"
#include <string_view>
#include <variant>
#include <vector>

namespace squi {
	struct ContextMenuItem {
		std::string_view text;
		std::variant<
			std::function<void()>,      // On click
			std::vector<ContextMenuItem>// Submenu
			>
			action{};
    	};

	struct ContextMenu {
		// Args
		Widget::Args widget;
		std::vector<ContextMenuItem> items;

		struct Storage {
			// Data
		};

		operator Child() const;
	};
}// namespace squi

#endif