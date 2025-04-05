#include "widgets/iconButton.hpp"
#include "align.hpp"
#include "fontIcon.hpp"
#include "text.hpp"
#include "wrapper.hpp"


using namespace squi;

squi::IconButton::operator squi::Child() const {
	Observable<Color> iconColorEvent;

	Child iconWidget = FontIcon{
		.textWidget{
			.onInit = [iconColorEvent](Widget &w) {
				observe(w, iconColorEvent, [&w](Color color) {
					w.as<Text::Impl>().setColor(color);
				});
			},
		},
		.icon = icon,
	};

	return Wrapper{
		.afterInit = [iconWidgetPtr = iconWidget->weak_from_this(), iconColorEvent](Widget &w) {
			observe(w, Button::State::stateEvent.of(w), [iconWidgetPtr, &w, iconColorEvent](ButtonState state) {
				if (auto iconWidget = iconWidgetPtr.lock()) {
					auto style = *Button::State::style.of(w);
					switch (state) {
						case ButtonState::resting:
							iconColorEvent.notify(style.textColor);
							break;
						case ButtonState::active:
							iconColorEvent.notify(style.textColorActive);
							break;
						case ButtonState::disabled:
							iconColorEvent.notify(style.textColorDisabled);
							break;
						case ButtonState::hovered:
							iconColorEvent.notify(style.textColorHover);
							break;
					}
				}
			});
			iconColorEvent.notify(Button::State::style.of(w)->textColor);
		},
		.child = Button{
			.widget = widget.withDefaultWidth(32.f),
			.style = style,
			.disabled = disabled,
			.onClick = onClick,
			.child = Align{.child = iconWidget},
		},
	};
}
