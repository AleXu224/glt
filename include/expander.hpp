#include "widget.hpp"
#include <optional>
#include <string_view>

namespace squi {
	struct Expander {
		// Args
		Widget::Args widget{};
		std::optional<char32_t> icon{};
		std::variant<std::string_view, Child> heading = "Heading";
		std::optional<std::string_view> caption{};
		Children actions{};
		Child expandedContent{};

		struct Storage {
			// Data
			bool expanded = false;
		};

		operator Child() const;
	};
}// namespace squi