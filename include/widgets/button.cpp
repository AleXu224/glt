#include "widgets/button.hpp"

#include "theme.hpp"

namespace squi {
	Button::Theme Button::Theme::Accent() {
		auto theme = ThemeManager::getTheme();
		auto accentColor = theme.accent;
		auto isLight = accentColor.isLight();

		return {
			.resting{.backgroundColor = accentColor, .borderColor = Color::white * 0.08f, .textColor = isLight ? Color::black : Color::white},
			.hovered{.backgroundColor = accentColor * 0.9f, .borderColor = Color::white * 0.08f, .textColor = isLight ? Color::black : Color::white},
			.active{.backgroundColor = accentColor * 0.8f, .borderColor = 0xFFFFFF14, .textColor = isLight ? Color::black * 0.5f : Color::white * 0.78f},
			.disabled{.backgroundColor = 0xFFFFFF28, .borderColor = Color::transparent, .textColor = isLight ? Color::white * 0.5f : Color::white * 0.36f},
		};
	}

	Button::Theme Button::Theme::Standard() {
		return {
			.resting{.backgroundColor = 0xFFFFFF0F, .borderColor = 0xFFFFFF18, .textColor = Color::white},
			.hovered{.backgroundColor = 0xFFFFFF15, .borderColor = 0xFFFFFF18, .textColor = Color::white},
			.active{.backgroundColor = 0xFFFFFF12, .borderColor = 0xFFFFFF12, .textColor = Color::white * 0.78f},
			.disabled{.backgroundColor = 0xFFFFFF12, .borderColor = 0xFFFFFF12, .textColor = Color::white * 0.36f},
		};
	}

	Button::Theme Button::Theme::Subtle() {
		return {
			.resting{.backgroundColor = Color::transparent, .borderWidth = 0.f, .textColor = Color::white},
			.hovered{.backgroundColor = 0xFFFFFF0F, .borderWidth = 0.f, .textColor = Color::white},
			.active{.backgroundColor = 0xFFFFFF0B, .borderWidth = 0.f, .textColor = Color::white * 0.78f},
			.disabled{.backgroundColor = Color::transparent, .borderWidth = 0.f, .textColor = Color::white * 0.36f},
		};
	}
}// namespace squi