#include "button.hpp"
#include "align.hpp"
#include "box.hpp"
#include "gestureDetector.hpp"
#include "memory"
#include "observer.hpp"
#include "text.hpp"

using namespace squi;

Button::operator squi::Child() const {
	Observable<ButtonState> stateEvent;
	Observable<std::string> updateTextEvent;

	Child textW = Text{
		.widget{
			.onInit = [updateTextEvent](Widget &w) {
				observe(w, updateTextEvent, [&w](const std::string &text) {
					w.as<Text::Impl>().setText(text);
				});
			},
		},
		.text = text,
		.color = disabled ? style.textColorDisabled : style.textColor,
	};

	Child ret = GestureDetector{
		.onClick = [onClick = onClick](GestureDetector::Event event) {
			auto &disabled = event.widget.customState.get<bool>("disabled");
			if (!disabled && onClick) {
				onClick(event);
			}
		},
		.onUpdate = [textRef = ChildRef(textW), stateEvent](GestureDetector::Event event) {
			auto &box = reinterpret_cast<Box::Impl &>(event.widget);
			auto &disabled = event.widget.customState.get<bool>("disabled");
			auto &style = event.widget.customState.get<ButtonStyle>("style");

			auto text = textRef.lock();

			auto &buttonState = event.widget.customState.get<ButtonState>("state");

			if (disabled) {
				box.setColor(style.colorDisabled);
				box.setBorderColor(style.borderColorDisabled);
				if (text) text->as<Text::Impl>().setColor(style.textColorDisabled);
				if (buttonState != ButtonState::disabled) stateEvent.notify(ButtonState::disabled);
				buttonState = ButtonState::disabled;
			} else if (event.state.focused) {
				box.setColor(style.colorActive);
				box.setBorderColor(style.borderColorActive);
				if (text) text->as<Text::Impl>().setColor(style.textColorActive);
				if (buttonState != ButtonState::active) stateEvent.notify(ButtonState::active);
				buttonState = ButtonState::active;
			} else if (event.state.hovered) {
				box.setColor(style.colorHover);
				box.setBorderColor(style.borderColorHover);
				if (text) text->as<Text::Impl>().setColor(style.textColorHover);
				if (buttonState != ButtonState::hovered) stateEvent.notify(ButtonState::hovered);
				buttonState = ButtonState::hovered;
			} else {
				box.setColor(style.color);
				box.setBorderColor(style.borderColor);
				if (text) text->as<Text::Impl>().setColor(style.textColor);
				if (buttonState != ButtonState::resting) stateEvent.notify(ButtonState::resting);
				buttonState = ButtonState::resting;
			}
		},
		.child{
			Box{
				.widget = widget.withDefaultWidth(Size::Shrink).withDefaultHeight(Size::Shrink).withDefaultPadding(Padding{6, 12, 6, 12}).withSizeConstraints(widget.sizeConstraints.withDefaultMinWidth(32.f).withDefaultMinHeight(32.f)),
				.color{style.color},
				.borderColor{style.borderColor},
				.borderWidth{style.borderWidth},
				.borderRadius{style.borderRadius},
				.borderPosition = style.borderPosition,
				.child = child ? child : Align{.child = textW},
			},
		},
	};

	ret->customState.add("style", style);
	ret->customState.add("disabled", disabled);
	ret->customState.add("state", ButtonState::resting);
	ret->customState.add("stateEvent", stateEvent);
	ret->customState.add("updateText", updateTextEvent);

	return ret;
}