#include "widget.hpp"
#include <optional>
#include <string_view>

namespace squi {
	struct Expander {
		// Args
		Widget::Args widget;
        std::optional<char32_t> icon{};
        std::string_view heading = "Heading";
        std::optional<std::string_view> caption{};
        Child child{};

		struct Storage {
			// Data
		};

		operator Child() const;
	};
}// namespace squi