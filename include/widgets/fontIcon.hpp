#pragma once

#include "core/core.hpp"

#include "widgets/text.hpp"


namespace squi {
	struct FontIcon : public StatelessWidget {
		Key key;
		float size = 16.f;
		float fontSize = 24.f;
		decltype(Text::font) font = FontStore::defaultIcons;
		Color color = Color::white;
		char32_t icon;

		Child build(const Element &);
	};
}// namespace squi