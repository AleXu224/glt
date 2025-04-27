#include "widgets/toggleButton.hpp"

#include "button.hpp"
#include "wrapper.hpp"


using namespace squi;

squi::ToggleButton::operator squi::Child() const {
	Observable<bool> stateEvent;

	auto accentTheme = ButtonStyle::Accent();

	auto getStyle = [modifyStyle = modifyStyle, accentTheme](bool active) {
		auto style = [&]() {
			if (active) return accentTheme;
			else
				return ButtonStyle::Standard();
		}();
		if (modifyStyle) modifyStyle(style, active);
		return style;
	};

	Child ret = Wrapper{
		.onInit = [stateEvent, getStyle](Widget &w) {
			observe("toggleButtonStateEventObs", w, stateEvent, [&w, getStyle](bool active) {
				Button::State::style.of(w) = getStyle(active);
			});
		},
		.child = Button{
			.widget = widget,
			.text = text,
			.style = getStyle(active),
			.disabled = disabled,
			.onClick = [stateEvent, onSwitch = onSwitch](GestureDetector::Event event) {
				auto &active = State::active.of(event.widget);
				active = !active;
				stateEvent.notify(active);
				if (onSwitch) onSwitch(active);
			},
		},
	};

	State::active.bind(ret, active);
	State::stateEvent.bind(ret, stateEvent);

	return ret;
}
