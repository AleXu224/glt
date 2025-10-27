#include "widgets/button.hpp"

#include "animatedText.hpp"
#include "theme.hpp"
#include "utils.hpp"
#include "widgets/animatedBox.hpp"
#include "widgets/gestureDetector.hpp"


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
			.active{.backgroundColor = Color::white * 0.0326f, .borderColor = 0xFFFFFF12, .textColor = Color::white * 0.78f},
			.disabled{.backgroundColor = Color::white * 0.0419f, .borderColor = 0xFFFFFF12, .textColor = Color::white * 0.36f},
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
	void Button::State::updateStatus() {
		ButtonStatus newStatus = ButtonStatus::resting;
		if (widget->disabled) {
			newStatus = ButtonStatus::disabled;
		} else if (isActive) {
			newStatus = ButtonStatus::active;
		} else if (isHovered) {
			newStatus = ButtonStatus::hovered;
		}

		if (newStatus != status) {
			setState([&]() {
				status = newStatus;
			});
			if (widget->onStatusChange) {
				widget->onStatusChange(newStatus);
			}
		}
	}
	[[nodiscard]] core::Child Button::State::getContent(const Style &style) const {
		return std::visit(
			utils::overloaded{
				[&](const std::string &text) -> Child {
					return AnimatedText{
						.widget{
							.alignment = Alignment::Center,
						},
						.text = text,
						.fontSize = style.textSize,
						.color = style.textColor,
					};
				},
				[](const Child &child) -> Child {
					return child;
				}
			},
			widget->content
		);
	}
	[[nodiscard]] core::Args Button::State::getArgs() const {
		// Make the button shrink to the contents by default
		auto args = widget->widget;
		args.width = args.width.value_or(Size::Shrink);
		args.height = args.height.value_or(Size::Shrink);
		args.padding = args.padding.value_or(Padding{12.f, 6.f});
		args.sizeConstraints = args.sizeConstraints.value_or(BoxConstraints{
			.minWidth = 32.f,
			.minHeight = 32.f,
		});
		return args;
	}
	core::Child Button::State::build(const Element &) {
		auto style = widget->theme.fromStatus(status);

		return Gesture{
			.onEnter = [this](const Gesture::State &) {
				isHovered = true;
				updateStatus();
			},
			.onLeave = [this](const Gesture::State &) {
				isHovered = false;
				updateStatus();
			},
			.onFocus = [this](const Gesture::State &) {
				isActive = true;
				updateStatus();
			},
			.onFocusLoss = [this](const Gesture::State &) {
				isActive = false;
				updateStatus();
			},
			.onClick = [this](const Gesture::State &) {
				if (widget->onClick) {
					widget->onClick();
				}
			},
			.child = AnimatedBox{
				.widget = getArgs(),
				.color = style.backgroundColor,
				.borderColor = style.borderColor,
				.borderWidth = style.borderWidth,
				.borderRadius = style.borderRadius,
				.borderPosition = style.borderPosition,
				.child = getContent(style),
			},
		};
	}
}// namespace squi