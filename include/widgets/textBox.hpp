#pragma once

#include "core/core.hpp"
#include "widgets/box.hpp"
#include "widgets/button.hpp"
#include "widgets/stack.hpp"
#include "widgets/textInput.hpp"


namespace squi {
	struct TextBox : StatefulWidget {
		// Args
		Key key;
		bool disabled = false;

		struct State : WidgetState<TextBox> {
			Button::Status status = Button::Status::resting;
			bool active = false;
			bool hovered = false;

			void updateStatus() {
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

			Child build(const Element &) override {
				static auto theme = []() {
					auto ret = Button::Theme::Standard();
					ret.active.textColor = Color::white;
					return ret;
				}();

				auto style = theme.fromStatus(status);

				return Gesture{
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
					.child = Box{
						.widget{
							.width = Size::Shrink,
							.height = 32.f,
							.padding = Padding{}.withHorizontal(12.f),
						},
						.color = style.backgroundColor,
						.borderColor = style.borderColor,
						.borderWidth = style.borderWidth,
						.borderRadius = 4.f,
						.borderPosition = style.borderPosition,
						.child = Stack{
							.children{
								TextInput{
									.widget{
										.alignment = Alignment::CenterLeft,
										.sizeConstraints = BoxConstraints{
											.minWidth = 100.f,
											.maxWidth = 200.f,
										},
									},
									.active = status == Button::Status::active,
								},
							},
						},
					},
				};
			}
		};
	};
}// namespace squi