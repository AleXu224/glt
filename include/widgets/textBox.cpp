#include "widgets/textBox.hpp"

#include "theme.hpp"
#include "widgets/animatedBox.hpp"
#include "widgets/column.hpp"
#include "widgets/stack.hpp"
#include "widgets/text.hpp"
#include "widgets/textArea.hpp"

namespace squi {
	void TextBox::State::updateStatus() {
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

	Child TextBox::State::build(const Element &element) {
		static auto theme = []() {
			auto ret = Button::Theme::Standard();
			ret.active.textColor = Color::white;
			ret.active.backgroundColor = Color::css(30, 30, 30, 0.7f);
			return ret;
		}();

		auto style = theme.fromStatus(status);

		return Column{
			.widget{
				.width = Size::Wrap,
				.height = Size::Wrap,
			},
			.spacing = 4.f,
			.children{
				Gesture{
					.onEnter = [this](const Gesture::State &) {
						hovered = true;
						updateStatus();
					},
					.onLeave = [this](const Gesture::State &) {
						hovered = false;
						updateStatus();
					},
					.onActive = [this](const Gesture::State &) {
						active = true;
						updateStatus();
					},
					.onInactive = [this](const Gesture::State &) {
						active = false;
						updateStatus();
					},
					.onUpdate = [this](const Gesture::State &state) {
						if (!state.active) return;
						if (widget->onArrowUp && state.isKeyPressedOrRepeat(GestureKey::up)) widget->onArrowUp();
						if (widget->onArrowDown && state.isKeyPressedOrRepeat(GestureKey::down)) widget->onArrowDown();
					},
					.child = AnimatedBox{
						.widget = widget->widget.withDefaults({
							.width = Size::Shrink,
							.height = widget->multiline ? SizeVariant(Size::Shrink) : 32.f,
							.sizeConstraints{
								.minWidth = 100.f,
								.minHeight = 32.f,
							},
						}),
						.color = style.backgroundColor,
						.borderColor = style.borderColor,
						.borderWidth = style.borderWidth,
						.borderRadius = style.borderRadius,
						.borderPosition = style.borderPosition,
						.child = Stack{
							.children{
								AnimatedBox{
									.color = Color::transparent,
									.borderColor = [&]() {
										if (!errorMessage.empty()) return Color(0xFF99A4FF);

										switch (status) {
											case Button::ButtonStatus::disabled:
												return Color::transparent;
											case Button::ButtonStatus::active:
												return Theme::of(element).accent;
											default:
												return Color::white * 0.54f;
										}
									}(),
									.borderWidth = BorderWidth{}.withBottom((status == Button::ButtonStatus::active ? 2.f : 1.f)),
									.borderRadius = style.borderRadius,
								},
								[&]() -> Child {
									if (widget->multiline)
										return TextArea{
											.widget{
												.alignment = Alignment::CenterLeft,
												.padding = Padding{12.f, 4.f},
											},
											.controller = widget->controller,
											.onTextChanged = [this](const std::string &text) {
												if (widget->validator) {
													auto err = widget->validator(text);
													setState([&]() {
														errorMessage = err.value_or("");
													});
												}
											},
											.active = status == Button::ButtonStatus::active,
										};
									else
										return TextInput{
											.widget{
												.alignment = Alignment::CenterLeft,
												.padding = Padding{12.f, 4.f},
											},
											.controller = widget->controller,
											.onTextChanged = [this](const std::string &text) {
												if (widget->validator) {
													auto err = widget->validator(text);
													setState([&]() {
														errorMessage = err.value_or("");
													});
												}
											},
											.onSubmit = widget->onSubmit,
											.active = status == Button::ButtonStatus::active,
										};
								}(),
							},
						},
					},
				},
				errorMessage.empty()//
					? Child{}
					: Text{
						  .text = errorMessage,
						  .color = 0xFF99A4FF,
					  },
			},
		};
	}
}// namespace squi