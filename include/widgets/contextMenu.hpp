#pragma once

#include "core/core.hpp"

#include <functional>
#include <variant>
#include <vector>

namespace squi {
	struct ContextMenu : StatefulWidget {
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

		using Item = std::variant<Button, Toggle, Divider>;

		// Args
		Key key;
		Args widget{};
		Key overlayKey;
		Key targetKey{};
		vec2 position{};
		std::function<void()> onClose;
		std::function<void(int64_t)> onSelectionChange;
		std::vector<Item> items;

		struct State : WidgetState<ContextMenu> {
			int64_t selectedIndex = -1;

			void widgetUpdated() override;

			void moveSelection(int64_t delta);
			void activateSelected();
			void close();

			Child build(const Element &) override;
		};
	};
}// namespace squi
