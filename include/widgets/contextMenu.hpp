#pragma once

#include "core/core.hpp"

namespace squi {
	struct ContextMenu : StatelessWidget {
		struct Button {
			std::string text{};
			std::function<void()> callback;
		};
		struct Toggle {
			std::string text{};
			bool value;
			std::function<void(bool)> callback;
		};
		struct Divider {};

		// Args
		Key key;
		vec2 position;
		std::function<void()> onClose;
		std::vector<std::variant<Button, Toggle, Divider>> items;

		[[nodiscard]] Child build(const Element &) const;
	};
}// namespace squi