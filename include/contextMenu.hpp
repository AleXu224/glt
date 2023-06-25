#ifndef SQUI_CONTEXTMENU_HPP
#define SQUI_CONTEXTMENU_HPP

#include "widget.hpp"
#include <memory>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace squi {
	struct ContextMenu {
		struct Item {
			struct Submenu {
				std::vector<Item> items;
			};
			struct Toggle {
				bool value;
				std::function<void(bool)> callback;
			};
			struct Divider{};
			
			std::string_view text;
			std::variant<std::function<void()>, Submenu, Toggle, Divider> content;
		};

		// Args
		vec2 position{};
		std::vector<Item> items;

		struct Storage {
			// Data
			ChildRef stack{};
			std::vector<Child> menusToAdd{};
			
			[[nodiscard]] ChildRef addMenu(const Child &menu);
			static void removeMenu(const ChildRef &menu);
		};

		operator Child() const;
	};
}// namespace squi

#endif