#include "textBox.hpp"
#include "box.hpp"
#include "column.hpp"
#include "gestureDetector.hpp"
#include "stack.hpp"
#include "text.hpp"
#include "textInput.hpp"
#include "wrapper.hpp"


#include "GLFW/glfw3.h"

using namespace squi;

using BoxInputState = TextBox::InputState;
using StObs = Observable<BoxInputState>;

struct Body {
	// Args
	squi::Widget::Args widget{};
	StObs stateUpdateObs;
	Observable<std::string> textUpdateObs{};
	VoidObservable requestOnSubmitCall{};
	VoidObservable selectAll{};
	std::function<void(std::string_view)> onChange{};
	std::function<void(std::string_view)> onSubmit{};
	TextBox::Style style;
	std::string_view text;

	operator squi::Child() const {
		return Box{
			.widget{
				.onInit = [stateUpdateObs = stateUpdateObs, style = style](Widget &w) {
					w.customState.add(stateUpdateObs.observe([&w, style](BoxInputState state) {
						auto &box = w.as<Box::Impl>();
						switch (state) {
							case squi::TextBox::InputState::resting: {
								box.setColor(style.rest);
								return;
							}
							case squi::TextBox::InputState::hovered: {
								box.setColor(style.hover);
								return;
							}
							case squi::TextBox::InputState::focused: {
								box.setColor(style.active);
								return;
							}
							case squi::TextBox::InputState::disabled: {
								box.setColor(style.disabled);
								return;
							}
						}
					}));
				},
			},
			.color{style.rest},
			.borderColor{style.border},
			.borderWidth{1.f},
			.borderRadius{4.f},
			.borderPosition = Box::BorderPosition::outset,
			.child = TextInput{
				.widget{
					.padding = Padding{11.f, 0.f},
					.onInit = [style = style, stateUpdateObs = stateUpdateObs, textUpdateObs = textUpdateObs, reqOnSubmitObs = requestOnSubmitCall, onSubmit = onSubmit](Widget &w) {
						w.customState.add(stateUpdateObs.observe([&w, style](BoxInputState state) {
							auto &input = w.as<TextInput::Impl>();
							switch (state) {
								case squi::TextBox::InputState::focused: {
									input.setActive(true);
									break;
								}
								default: {
									input.setActive(false);
									break;
								}
							}
							switch (state) {
								case squi::TextBox::InputState::disabled: {
									input.setColor(style.textDisabled);
									break;
								}
								default: {
									input.setColor(style.text);
									break;
								}
							}
						}));
						w.customState.add(textUpdateObs.observe([&w](std::string_view newText) {
							auto &input = w.as<TextInput::Impl>();
							input.setText(newText);
						}));
						w.customState.add(reqOnSubmitObs.observe([&w, onSubmit = onSubmit]() {
							auto &input = w.as<TextInput::Impl>();
							if (onSubmit) onSubmit(input.getText());
						}));
					},
				},
				.fontSize = 14.0f,
				.text = text,
				.onTextChanged = onChange,
				.color{style.text},
				.controller{
					.selectAll = selectAll,
				},
			},
		};
	}
};

struct Underline {
	// Args
	StObs stateUpdateObs{};
	TextBox::Style style;

	operator squi::Child() const {
		return Box{
			.widget{
				.onInit = [stateUpdateObs = stateUpdateObs, style = style](Widget &w) {
					w.flags.isInteractive = false;
					w.customState.add(stateUpdateObs.observe([&w, style](BoxInputState state) {
						auto &box = w.as<Box::Impl>();
						switch (state) {
							case squi::TextBox::InputState::resting:
							case squi::TextBox::InputState::hovered:
							case squi::TextBox::InputState::disabled: {
								box.setBorderColor(style.bottomBorder);
								box.setBorderWidth({0.f, 0.f, 1.f, 0.f});
								return;
							}
							case squi::TextBox::InputState::focused: {
								box.setBorderColor(style.bottomBorderActive);
								box.setBorderWidth({0.f, 0.f, 2.f, 0.f});
								return;
							}
						}
					}));
				},
			},
			.color{0.f, 0.f, 0.f, 0.f},
			.borderColor{style.bottomBorder},
			.borderWidth{0.f, 0.f, 1.f, 0.f},
			.borderRadius{4.f},
		};
	}
};

squi::TextBox::operator squi::Child() const {
	auto storage = std::make_shared<Storage>();
	storage->disabled = disabled;
	storage->stateObserver = controller.stateObserver;
	storage->disableObs = controller.disable.observe([&storage = *storage](bool newDisabled) {
		if (storage.disabled != newDisabled) {
			storage.disabled = newDisabled;
			storage.stateObserver.notify(storage.getState());
		}
	});
	storage->focusObs = controller.focus.observe([&storage = *storage, onSubmitFunc = controller.onSubmit](bool newFocus) {
		if (storage.focused != newFocus) {
			storage.focused = newFocus;
			storage.stateObserver.notify(storage.getState());

			if (!newFocus) {
				storage.requestOnSubmitCall.notify();
			}
		}
	});

	Observable<std::string_view> onChangeObs{};

	return Wrapper{
		.afterInit = [storage](Widget &w) {
			// For outside use
			w.customState.add(storage);
			storage->stateObserver.notify(storage->getState());
		},
		.child = GestureDetector{
			.onEnter = [storage](auto) {
				storage->hovered = true;
				storage->stateObserver.notify(storage->getState());
			},
			.onLeave = [storage](auto) {
				storage->hovered = false;
				storage->stateObserver.notify(storage->getState());
			},
			.onUpdate = [storage](GestureDetector::Event event) {
				if (event.state.isKey(GLFW_MOUSE_BUTTON_1, GLFW_PRESS)) {
					if (event.state.focused) storage->focusObs.notifyOthers(true);
					if (event.state.focusedOutside) storage->focusObs.notifyOthers(false);
				}

				if (storage->focused && (event.state.isKeyPressedOrRepeat(GLFW_KEY_ENTER) || event.state.isKeyPressedOrRepeat(GLFW_KEY_ESCAPE))) {
					storage->focusObs.notifyOthers(false);
				}
			},
			.child = Column{
				.widget{widget.withDefaultHeight(Size::Shrink).withDefaultWidth(Size::Shrink).withSizeConstraints(SizeConstraints{.minWidth = 124.f})},
				.children{
					Stack{
						.widget{
							.height = 32.f,
						},
						.children{
							Body{
								.stateUpdateObs = storage->stateObserver,
								.textUpdateObs = controller.updateText,
								.requestOnSubmitCall = storage->requestOnSubmitCall,
								.selectAll = controller.selectAll,
								.onChange = [onChangeObs, onChange = controller.onChange, validator = controller.validator](std::string_view str) {
									onChangeObs.notify(str);
									if (validator && !validator(str).valid) return;
									if (onChange) onChange(str);
								},
								.onSubmit = controller.onSubmit,
								.style = style,
								.text = text,
							},
							Underline{
								.stateUpdateObs = storage->stateObserver,
								.style = style,
							},
						},
					},
					// Error text
					Text{
						.widget{
							.onInit = [onChangeObs, validator = controller.validator](Widget &w) {
								w.flags.visible = false;
								w.customState.add(onChangeObs.observe([&w, validator](std::string_view str) {
									if (!validator) return;
									auto ret = validator(str);
									auto &text = w.as<Text::Impl>();
									if (ret.valid) {
										text.flags.visible = false;
										return;
									}

									text.flags.visible = true;
									text.setText(ret.message);
								}));
							},
						},
						.text = "UwU",
						.lineWrap = true,
						.color{0xFF99A4FF},
					},
				},
			},
		},
	};
}

BoxInputState squi::TextBox::Storage::getState() const {
	if (disabled) return BoxInputState::disabled;
	if (focused) return BoxInputState::focused;
	if (hovered) return BoxInputState::hovered;
	return BoxInputState::resting;
}
