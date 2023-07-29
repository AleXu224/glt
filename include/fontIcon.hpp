#ifndef SQUI_FONTICON_HPP
#define SQUI_FONTICON_HPP

#include "color.hpp"
#include "margin.hpp"
#include "widget.hpp"
#include <string_view>

namespace squi {
	struct FontIcon {
		// Args
		Margin margin{};
		std::string_view icon;
		std::string_view font = R"(C:\Windows\Fonts\segmdl2.ttf)";
		float size = 14.f;
		Color color = Color::HEX(0xFFFFFFFF);

		operator Child() const;
	};
}// namespace squi

#endif