#include "theme.hpp"

#include "core/app.hpp"
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
		return element.getApp()->theme;
	}

	Theme Theme::of(const core::Element *element) {
		assert(element);
		return of(*element);
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