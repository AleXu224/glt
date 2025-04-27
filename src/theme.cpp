#include "theme.hpp"

#ifdef _WIN32
#include "guiddef.h"
#include "winrt/windows.ui.viewmanagement.h"// IWYU pragma: keep
#endif

using namespace squi;
using namespace winrt;
using namespace Windows::UI::ViewManagement;

std::optional<Color> squi::ThemeManager::getSystemAccentColor() {
#ifndef _WIN32
	return std::nullopt;
#else
	UISettings const ui_settings{};
	auto const accent_color{ui_settings.GetColorValue(UIColorType::AccentLight2)};

	return Color::css(accent_color.R, accent_color.G, accent_color.B);
#endif
}
