#pragma once

#include "animatedText.hpp"
#include "color.hpp"
#include "core/core.hpp"
#include "utils.hpp"
#include "widgets/animatedBox.hpp"
#include "widgets/gestureDetector.hpp"


namespace squi {
	struct Button : StatefulWidget {
		enum class ButtonStatus : uint8_t {
			resting,
			hovered,
			active,
			disabled,
		};

		struct Style {
			Color backgroundColor{};
			Color borderColor{};
			Box::BorderPosition borderPosition = Box::BorderPosition::inset;
			BorderRadius borderRadius{4.0f};
			BorderWidth borderWidth{1.0f};
			Color textColor{0x000000FF};
			float textSize{14.0f};
		};

		struct Theme {
			Style resting;
			Style hovered;
			Style active;
			Style disabled;

			static Theme Accent();
			static Theme Standard();
			static Theme Subtle();

			auto &&fromStatus(this auto &&self, ButtonStatus status) {
				switch (status) {
					case ButtonStatus::resting:
						return self.resting;
					case ButtonStatus::hovered:
						return self.hovered;
					case ButtonStatus::active:
						return self.active;
					case ButtonStatus::disabled:
						return self.disabled;
				}
				throw std::runtime_error("Invalid button status");
			}
		};

		// Args
		Key key;
		Args widget{};
		Theme theme = Theme::Standard();
		bool disabled;
		std::function<void(ButtonStatus)> onStatusChange{};
		std::function<void()> onClick{};
		std::variant<std::string, Child> content = "Button";

		struct State : WidgetState<Button> {
			ButtonStatus status = ButtonStatus::resting;
			bool isHovered = false;
			bool isActive = false;

			void updateStatus() {
				ButtonStatus newStatus = ButtonStatus::resting;
				if (widget->disabled) {
					newStatus = ButtonStatus::disabled;
				} else if (isActive) {
					newStatus = ButtonStatus::active;
				} else if (isHovered) {
					newStatus = ButtonStatus::hovered;
				}

				if (newStatus != status) {
					setState([&]() {
						status = newStatus;
					});
					if (widget->onStatusChange) {
						widget->onStatusChange(newStatus);
					}
				}
			}

			[[nodiscard]] Child getContent(const Style &style) const {
				return std::visit(
					utils::overloaded{
						[&](const std::string &text) -> Child {
							return AnimatedText{
								.widget{
									.alignment = Alignment::Center,
								},
								.text = text,
								.fontSize = style.textSize,
								.color = style.textColor,
							};
						},
						[](const Child &child) -> Child {
							return child;
						}
					},
					widget->content
				);
			}

			[[nodiscard]] Args getArgs() const {
				// Make the button shrink to the contents by default
				auto args = widget->widget;
				args.width = args.width.value_or(Size::Shrink);
				args.height = args.height.value_or(Size::Shrink);
				args.padding = args.padding.value_or(Padding{12.f, 6.f});
				args.sizeConstraints = args.sizeConstraints.value_or(BoxConstraints{
					.minWidth = 32.f,
					.minHeight = 32.f,
				});
				return args;
			}

			void widgetUpdated() override {
				updateStatus();
			}

			Child build(const Element &) override {
				auto style = widget->theme.fromStatus(status);

				return Gesture{
					.onEnter = [this](const Gesture::State &) {
						isHovered = true;
						updateStatus();
					},
					.onLeave = [this](const Gesture::State &) {
						isHovered = false;
						updateStatus();
					},
					.onFocus = [this](const Gesture::State &) {
						isActive = true;
						updateStatus();
					},
					.onFocusLoss = [this](const Gesture::State &) {
						isActive = false;
						updateStatus();
					},
					.onClick = [this](const Gesture::State &) {
						if (widget->onClick) {
							widget->onClick();
						}
					},
					.child = AnimatedBox{
						.widget = getArgs(),
						.color = style.backgroundColor,
						.borderColor = style.borderColor,
						.borderWidth = style.borderWidth,
						.borderRadius = style.borderRadius,
						.borderPosition = style.borderPosition,
						.child = getContent(style),
					},
				};
			}
		};
	};
}// namespace squi