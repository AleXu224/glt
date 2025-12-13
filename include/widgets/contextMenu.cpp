#include "contextMenu.hpp"

#include "slideIn.hpp"
#include "widgets/animatedText.hpp"
#include "widgets/box.hpp"
#include "widgets/button.hpp"
#include "widgets/card.hpp"
#include "widgets/column.hpp"
#include "widgets/fontIcon.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/navigator.hpp"
#include "widgets/offset.hpp"
#include "widgets/row.hpp"
#include "widgets/stack.hpp"

#include "utils.hpp"

namespace squi {
	struct ContextMenuButton : StatelessWidget {
		// Args
		Key key;
		std::string label;
		std::function<void()> onClick;
		std::function<void()> onClose;

		[[nodiscard]] Child build(const Element &element) const {
			auto buttonTheme = Button::Theme::Subtle();
			buttonTheme.resting.alignment = Alignment::CenterLeft;
			buttonTheme.hovered.alignment = Alignment::CenterLeft;
			buttonTheme.active.alignment = Alignment::CenterLeft;
			buttonTheme.disabled.alignment = Alignment::CenterLeft;

			return Button{
				.widget{
					.width = Size::Expand,
					.height = 28.f,
					.margin = Margin{5.f, 2.f},
					.padding = Padding{4.f, 2.f},
				},
				.theme = buttonTheme,
				.onClick = [this, &element]() mutable {
					if (this->onClick) this->onClick();
					if (this->onClose) this->onClose();
					Navigator::of(element).popOverlay();
				},
				.content = label,
			};
		}
	};

	struct ContextMenuToggle : StatefulWidget {
		// Args
		Key key;
		std::string label;
		std::function<void(bool)> onClick;
		bool currentValue;
		std::function<void()> onClose;

		struct State : WidgetState<ContextMenuToggle> {
			Button::ButtonStatus status = Button::ButtonStatus::resting;

			Child build(const Element &element) override {
				auto buttonTheme = Button::Theme::Subtle();
				buttonTheme.resting.alignment = Alignment::CenterLeft;
				buttonTheme.hovered.alignment = Alignment::CenterLeft;
				buttonTheme.active.alignment = Alignment::CenterLeft;
				buttonTheme.disabled.alignment = Alignment::CenterLeft;

				auto style = buttonTheme.fromStatus(status);

				return Button{
					.widget{
						.width = Size::Expand,
						.height = 28.f,
						.margin = Margin{5.f, 2.f},
						.padding = Padding{4.f, 2.f},
					},
					.theme = buttonTheme,
					.onStatusChange = [&](Button::ButtonStatus status) {
						setState([&]() {
							this->status = status;
						});
					},
					.onClick = [&]() {
						if (widget->onClick) widget->onClick(!widget->currentValue);
						if (widget->onClose) widget->onClose();
						Navigator::of(element).popOverlay();
					},
					.content = Row{
						.crossAxisAlignment = Flex::Alignment::center,
						.spacing = 4.f,
						.children{
							widget->currentValue//
								? Child(FontIcon{
									  .color = style.textColor,
									  .icon = 0xe5ca,
								  })
								: Child(Box{
									  .widget{
										  .width = style.textSize,
										  .height = style.textSize,
									  },
									  .color = Color::transparent,
								  }),
							AnimatedText{
								.text = widget->label,
								.fontSize = style.textSize,
								.color = style.textColor,
							},
						},
					},
				};
			}
		};
	};

	[[nodiscard]] core::Child ContextMenu::build(const Element &element) const {
		return Stack{
			.children{
				Gesture{
					.onClick = [&](const Gesture::State &) {
						if (onClose) onClose();
						Navigator::of(element).popOverlay();
					},
					.child = Box{
						.color = Color::transparent,
					},
				},
				Offset{
					.calculateContentBounds = [&](auto, const SingleChildRenderObject &renderObject) -> Rect {
						auto rect = renderObject.getLayoutRect();
						return Rect::fromPosSize(position, rect.size());
					},
					.child = SlideIn{
						.child = Card{
							.widget{
								.width = Size::Shrink,
								.height = Size::Shrink,
								.sizeConstraints = BoxConstraints{
									.minWidth = 100.f,
								},
								.padding = Padding{1.f, 4.f},
							},
							.child = Column{
								.children = [this]() {
									Children ret;

									for (const auto &item: items) {
										std::visit(
											utils::overloaded{
												[&](const ContextMenu::Button &button) {
													ret.emplace_back(ContextMenuButton{
														.label = button.text,
														.onClick = button.callback,
														.onClose = this->onClose,
													});
												},
												[&](const ContextMenu::Toggle &button) {
													ret.emplace_back(ContextMenuToggle{
														.label = button.text,
														.onClick = button.callback,
														.currentValue = button.value,
														.onClose = this->onClose,
													});
												},
												[&](const ContextMenu::Divider &) {
													ret.emplace_back(Box{
														.widget{
															.width = Size::Expand,
															.height = 1.f,
															.margin = Margin{1.f}.withBottom(2.f),
														},
														.color = Color::white * 0.0837f,
													});
												},
											},
											item
										);
									}

									return ret;
								}(),
							},
						},
					},
				},
			},
		};
	}
}// namespace squi