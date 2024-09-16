#pragma once

#include "color.hpp"
#include "fontStore.hpp"
#include "margin.hpp"
#include "widget.hpp"
#include <memory>
#include <variant>

namespace squi {
	struct FontIcon {
		// Args
		Widget::Args widget{};
		Widget::Args textWidget{};
		Margin margin{};
		char32_t icon = U'\0';
		// FIXME: add a default Icon font
		std::variant<FontProvider, std::shared_ptr<FontStore::Font>> font = FontProvider::fromFile(R"(C:\Windows\Fonts\segmdl2.ttf)");
		float size = 14.f;
		Color color = 0xFFFFFFFF;

		operator Child() const;
	};
}// namespace squi