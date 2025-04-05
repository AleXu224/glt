#include "widget.hpp"
#include <string_view>

namespace squi {
	struct Expander {
		// Args
		Widget::Args widget{};
		std::variant<char32_t, Child> icon = Child{};
		std::variant<std::string_view, Child> heading = "Heading";
		std::variant<std::string_view, Child> caption = Child{};
		bool alwaysExpanded = false;
		Children actions{};
		Child expandedContent{};

		struct Storage {
			// Data
			bool expanded = false;
		};

		operator Child() const;
	};
}// namespace squi