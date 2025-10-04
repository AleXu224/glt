#include "widgets/textBox.hpp"

#include "theme.hpp"
#include "widgets/column.hpp"
#include "widgets/stack.hpp"
#include "widgets/text.hpp"

namespace squi {
	void TextBox::State::updateStatus() {
		Button::Status newStatus = Button::Status::resting;
		if (widget->disabled) {
			newStatus = Button::Status::disabled;
		} else if (active) {
			newStatus = Button::Status::active;
		} else if (hovered) {
			newStatus = Button::Status::hovered;
		}

		if (newStatus != status) {
			setState([&]() {
				status = newStatus;
			});
		}
	}

	Child TextBox::State::build(const Element &) {
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
					.child = AnimatedBox{
						.widget{
							.width = Size::Shrink,
							.height = 32.f,
						},
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
											case Button::Status::disabled:
												return Color::transparent;
											case Button::Status::active:
												return ThemeManager::getTheme().accent;
											default:
												return Color::white * 0.54f;
										}
									}(),
									.borderWidth = BorderWidth{}.withBottom((status == Button::Status::active ? 2.f : 1.f)),
									.borderRadius = style.borderRadius,
								},
								TextInput{
									.widget{
										.alignment = Alignment::CenterLeft,
										.sizeConstraints = BoxConstraints{
											.minWidth = 100.f,
											.maxWidth = 200.f,
										},
										.margin = Margin{}.withHorizontal(12.f),
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
									.active = status == Button::Status::active,
								},
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