#include "iconButton.hpp"
#include "widgets/button.hpp"
#include "widgets/fontIcon.hpp"

namespace squi {
	core::Child IconButton::State::build(const Element &) {
		return Button{
			.widget = [&]() {
				auto args = widget->widget;
				args.width = args.width.value_or(32.f);
				return args;
			}(),
			.theme = widget->theme,
			.onStatusChange = [this](Button::ButtonStatus status) {
				setState([this, status]() {
					this->status = status;
				});
			},
			.onClick = widget->onClick,
			.child = FontIcon{
				.size = 16.f,
				.color = widget->theme.fromStatus(this->status).textColor,
				.icon = widget->icon,
			},
		};
	}
}// namespace squi