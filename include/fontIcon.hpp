#pragma once

#include "color.hpp"
#include "fontStore.hpp"
#include "margin.hpp"
#include "widget.hpp"
#include <memory>
#include <string_view>
#include <variant>

namespace squi {
	struct FontIcon {
		// Args
		Widget::Args widget{};
		Margin margin{};
		char32_t icon = U'\0';
		// FIXME: add a default Icon font
		std::variant<std::string_view, std::shared_ptr<FontStore::Font>> font = R"(C:\Windows\Fonts\segmdl2.ttf)";
		float size = 14.f;
		Color color = 0xFFFFFFFF;

		operator Child() const;
	};
}// namespace squi