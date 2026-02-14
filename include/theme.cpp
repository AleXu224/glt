#include "theme.hpp"

#include "widgets/themeOverride.hpp"

#ifdef _WIN32
#include "guiddef.h"
#include "winrt/windows.ui.viewmanagement.h"// IWYU pragma: keep
using namespace winrt;
using namespace Windows::UI::ViewManagement;
#endif

namespace squi {
	Theme Theme::of(const core::Element &element) {
		auto themeOverride = ThemeOverride::of(element);
		if (themeOverride) {
			return themeOverride->widget->theme;
		}
		return Theme{};
	}

	Theme Theme::of(const core::Element *element) {
		if (element) return of(*element);
		return Theme{};
	}
	std::optional<Color> Theme::getSystemAccentColor() {
#ifndef _WIN32
		return std::nullopt;
#else
		UISettings const ui_settings{};
		auto const accent_color{ui_settings.GetColorValue(UIColorType::AccentLight2)};

		return Color::css(accent_color.R, accent_color.G, accent_color.B);
#endif
	}
}// namespace squi