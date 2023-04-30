#ifndef SQUI_CONTEXTMENU_HPP
#define SQUI_CONTEXTMENU_HPP

#include "widget.hpp"
#include <memory>
#include <string_view>
#include <unordered_map>
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
	struct ContextMenuDivider{};

	struct ContextMenu {
		// Args
		vec2 position{};
		std::vector<ContextMenuItem> items;

		struct Storage {
			// Data
			uint32_t id = 0;
			std::unordered_map<uint32_t, std::shared_ptr<Widget>> menus{};
			std::vector<Child> menusToAdd{};

			std::optional<std::shared_ptr<Widget>> getMenu(uint32_t id);
			uint32_t addMenu(const Child &menu);
			void removeMenu(uint32_t id);
		};

		operator Child() const;
	};
}// namespace squi

#endif