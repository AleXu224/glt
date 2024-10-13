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
		std::variant<FontProvider, std::shared_ptr<FontStore::Font>> font = FontStore::defaultIconsFilled;
		// The size the icon will take
		float size = 16.f;
		// Some fonts are not sized too well and may require a different size
		// to fit nicely. An example of this is Material Icons, which in order
		// to get a 16x16px icon you actually need a font size of 24px
		float iconSize = 24.f;
		Color color = 0xFFFFFFFF;

		operator Child() const;
	};
}// namespace squi