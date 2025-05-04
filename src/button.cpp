#include "button.hpp"
#include "align.hpp"
#include "box.hpp"
#include "gestureDetector.hpp"
#include "memory"
#include "observer.hpp"
#include "text.hpp"
#include "theme.hpp"
#include "wrapper.hpp"


using namespace squi;

namespace {
	void updateStyle(Widget &w, const ChildRef &textRef, const ButtonState &state) {
		auto &box = w.as<Box::Impl>();
		const auto &style = Button::State::style.of(w);
		auto text = textRef.lock();
		switch (state) {
			case ButtonState::disabled: {
				box.setColor(style->colorDisabled);
				box.setBorderColor(style->borderColorDisabled);
				if (text) text->as<Text::Impl>().setColor(style->textColorDisabled);
				break;
			}
			case ButtonState::active: {
				box.setColor(style->colorActive);
				box.setBorderColor(style->borderColorActive);
				if (text) text->as<Text::Impl>().setColor(style->textColorActive);
				break;
			}
			case ButtonState::hovered: {
				box.setColor(style->colorHover);
				box.setBorderColor(style->borderColorHover);
				if (text) text->as<Text::Impl>().setColor(style->textColorHover);
				break;
			}
			case ButtonState::resting: {
				box.setColor(style->color);
				box.setBorderColor(style->borderColor);
				if (text) text->as<Text::Impl>().setColor(style->textColor);
				break;
			}
		}
	}
}// namespace

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

	Child ret = Wrapper{
		.onInit = [stateEvent, textRef = ChildRef(textW)](Widget &w) {
			observe("stateEventListener", w, stateEvent, [&w, textRef](ButtonState state) {
				updateStyle(w, textRef, state);
			});
		},
		.child = GestureDetector{
			.onClick = [onClick = onClick](GestureDetector::Event event) {
				auto &disabled = event.widget.customState.get<bool>("disabled");
				if (!disabled && onClick) {
					onClick(event);
				}
			},
			.onUpdate = [stateEvent](GestureDetector::Event event) {
				auto &disabled = State::disabled.of(event.widget);
				auto &buttonState = State::state.of(event.widget);

				if (disabled) {
					if (buttonState != ButtonState::disabled) stateEvent.notify(ButtonState::disabled);
					buttonState = ButtonState::disabled;
				} else if (event.state.focused) {
					if (buttonState != ButtonState::active) stateEvent.notify(ButtonState::active);
					buttonState = ButtonState::active;
				} else if (event.state.hovered) {
					if (buttonState != ButtonState::hovered) stateEvent.notify(ButtonState::hovered);
					buttonState = ButtonState::hovered;
				} else {
					if (buttonState != ButtonState::resting) stateEvent.notify(ButtonState::resting);
					buttonState = ButtonState::resting;
				}
			},
			.child{
				Box{
					.widget = widget.withDefaultWidth(Size::Shrink).withDefaultHeight(Size::Shrink).withDefaultPadding(Padding{6, 12, 6, 12}).withSizeConstraints(widget.sizeConstraints.withDefaultMinWidth(32.f).withDefaultMinHeight(32.f)),
					.color{disabled ? style.colorDisabled : style.color},
					.borderColor{style.borderColor},
					.borderWidth{style.borderWidth},
					.borderRadius{style.borderRadius},
					.borderPosition = style.borderPosition,
					.child = child ? child : Align{.child = textW},
				},
			},
		},
	};

	Button::State::style.bind(
		ret,
		Stateful<ButtonStyle, StateImpact::RedrawNeeded>(
			[textRef = ChildRef(textW), stateEvent](Widget &w, ButtonStyle) {
				stateEvent.notify(State::state.of(w));
			},
			ret.get(), style
		)
	);
	Button::State::disabled.bind(ret, disabled);
	Button::State::state.bind(ret, ButtonState::resting);
	Button::State::stateEvent.bind(ret, stateEvent);
	Button::State::updateText.bind(ret, updateTextEvent);

	return ret;
}

ButtonStyle squi::ButtonStyle::Accent() {
	auto theme = ThemeManager::getTheme();

	auto ret = ButtonStyle{
		.color = theme.accent,
	};

	if (!theme.accent.isLight()) {
		ret.textColor = Color{0xFFFFFFFF};
		ret.textColorHover = Color{0xFFFFFFFF};
		ret.textColorActive = Color{1.f, 1.f, 1.f, 0.78f};
		ret.textColorDisabled = Color{1.f, 1.f, 1.f, 0.36};
	}

	return ret;
}

ButtonStyle squi::ButtonStyle::Standard() {
	return ButtonStyle{
		.color{0xFFFFFF0F},
		.colorHover{0xFFFFFF15},
		.colorActive{0xFFFFFF12},
		.colorDisabled{0xFFFFFF12},
		.borderColor{0xFFFFFF18},
		.borderColorHover{0xFFFFFF18},
		.borderColorActive{0xFFFFFF12},
		.borderColorDisabled{0xFFFFFF12},
		.borderPosition = Box::BorderPosition::outset,
		.textColor{0xFFFFFFFF},
		.textColorHover{0xFFFFFFFF},
		.textColorActive{1.f, 1.f, 1.f, 0.78f},
		.textColorDisabled{1.f, 1.f, 1.f, 0.36},
	};
}

ButtonStyle squi::ButtonStyle::Subtle() {
	return ButtonStyle{
		.color{0xFFFFFF00},
		.colorHover{0xFFFFFF0F},
		.colorActive{0xFFFFFF0B},
		.colorDisabled{0xFFFFFF00},
		.borderWidth = 0.0f,
		.textColor{0xFFFFFFFF},
		.textColorHover{0xFFFFFFFF},
		.textColorActive{1.f, 1.f, 1.f, 0.78f},
		.textColorDisabled{1.f, 1.f, 1.f, 0.36},
	};
}
