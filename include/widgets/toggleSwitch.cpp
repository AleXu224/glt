#include "toggleSwitch.hpp"
#include "theme.hpp"
#include "widgets/animatedBox.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/row.hpp"


namespace squi {
	void ToggleSwitch::State::updateStatus() {
		Button::ButtonStatus newStatus = Button::ButtonStatus::resting;
		if (widget->disabled) {
			newStatus = Button::ButtonStatus::disabled;
		} else if (active) {
			newStatus = Button::ButtonStatus::active;
		} else if (hovered) {
			newStatus = Button::ButtonStatus::hovered;
		}

		if (newStatus != status) {
			setState([&]() {
				status = newStatus;
			});
		}
	}

	core::Child ToggleSwitch::State::build(const Element &element) {
		auto usedTheme = widget->active ? getActiveTheme() : getTheme();
		auto style = usedTheme.fromStatus(status);

		return Gesture{
			.onEnter = [this](const Gesture::State &) {
				hovered = true;
				updateStatus();
			},
			.onLeave = [this](const Gesture::State &) {
				hovered = false;
				updateStatus();
			},
			.onFocus = [this](const Gesture::State &) {
				active = true;
				updateStatus();
			},
			.onFocusLoss = [this](const Gesture::State &) {
				active = false;
				updateStatus();
			},
			.onClick = [this](const Gesture::State &) {
				if (widget->disabled) return;
				if (widget->onToggle) widget->onToggle(!widget->active);
			},
			.child = Row{
				.children{
					AnimatedBox{
						.widget{
							.width = 40.f,
							.height = 20.f,
							.padding = Padding{hovered && !widget->disabled ? 3.f : 4.f, 0.f},
						},
						.color = style.backgroundColor,
						.borderColor = style.borderColor,
						.borderWidth = 1.f,
						.borderRadius = 10.f,
						.borderPosition = Box::BorderPosition::outset,
						.child = AnimatedBox{
							.widget{
								.width = active && !widget->disabled//
										   ? 17.f
										   : hovered && !widget->disabled ? 14.f : 12.f,
								.height = (hovered || active) && !widget->disabled ? 14.f : 12.f,
								.alignment = widget->active ? Alignment::CenterRight : Alignment::CenterLeft,
							},
							.color = style.textColor,
							.borderRadius = hovered && !widget->disabled ? 7.f : 6.f,
						},
					},
				},
			},
		};
	}
	Button::Theme ToggleSwitch::State::getTheme() {
		return {
			.resting{.backgroundColor = Color::black * 0.1f, .borderColor = Color::rgba(255, 255, 255, 0.6047), .textColor = Color::rgba(255, 255, 255, 0.786)},
			.hovered{.backgroundColor = Color::rgba(255, 255, 255, 0.0419), .borderColor = Color::rgba(255, 255, 255, 0.6047), .textColor = Color::rgba(255, 255, 255, 0.786)},
			.active{.backgroundColor = Color::rgba(255, 255, 255, 0.0698), .borderColor = Color::rgba(255, 255, 255, 0.6047), .textColor = Color::rgba(255, 255, 255, 0.786)},
			.disabled{.backgroundColor = Color::rgba(255, 255, 255, 0), .borderColor = Color::rgba(255, 255, 255, 0.1581), .textColor = Color::rgba(255, 255, 255, 0.3628)},
		};
	}
	Button::Theme ToggleSwitch::State::getActiveTheme() {
		auto accent = ThemeManager::getTheme().accent;

		return {
			.resting{.backgroundColor = accent, .borderColor = accent, .textColor = Color::black},
			.hovered{.backgroundColor = accent * 0.9f, .borderColor = accent * 0.9f, .textColor = Color::black},
			.active{.backgroundColor = accent * 0.8f, .borderColor = accent * 0.8f, .textColor = Color::black},
			.disabled{.backgroundColor = Color::rgba(255, 255, 255, 0.1581), .borderColor = Color::rgba(255, 255, 255, 0.1581), .textColor = Color::rgba(255, 255, 255, 0.5302)},
		};
	}
}// namespace squi