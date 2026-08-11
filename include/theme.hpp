#pragma once

#include "color.hpp"
#include "core/forwards.hpp"

namespace squi {
	struct Theme {
		Color accent = 0x60CDFFFF;

		static Theme of(const core::Element &element);
		static Theme of(const core::Element *element);
		static Theme of(const core::WidgetStateBase *state);

		static std::optional<Color> getSystemAccentColor();
	};
}// namespace squi