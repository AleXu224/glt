#pragma once

#include "color.hpp"

namespace squi {
	struct Theme {
		Color accent = 0x60CDFFFF;

		static Theme of(const core::Element &element);
		static Theme of(const core::Element *element);

		static std::optional<Color> getSystemAccentColor();
	};
}// namespace squi