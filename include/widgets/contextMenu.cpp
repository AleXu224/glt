#include "contextMenu.hpp"

#include "core/app.hpp"
#include "slideIn.hpp"
#include "utils.hpp"
#include "widgets/animatedText.hpp"
#include "widgets/box.hpp"
#include "widgets/button.hpp"
#include "widgets/card.hpp"
#include "widgets/fontIcon.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/navigator.hpp"
#include "widgets/offset.hpp"
#include "widgets/row.hpp"
#include "widgets/scrollview.hpp"
#include "widgets/stack.hpp"

namespace squi {
	struct ContextMenuButton : StatelessWidget {
		// Args
		Key key;
		Key overlayKey;
		std::string label;
		bool highlighted = false;
		std::function<void()> onClick;
		std::function<void()> onClose;

		[[nodiscard]] Child build(const Element &element) const {
			auto buttonTheme = Button::Theme::Subtle();
			buttonTheme.resting.alignment = Alignment::CenterLeft;
			buttonTheme.hovered.alignment = Alignment::CenterLeft;
			buttonTheme.active.alignment = Alignment::CenterLeft;
			buttonTheme.disabled.alignment = Alignment::CenterLeft;

			if (highlighted) {
				buttonTheme.resting.backgroundColor = buttonTheme.hovered.backgroundColor;
			}

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
					Navigator::of(element).popOverlay(overlayKey);
				},
				.child = label,
			};
		}
	};

	struct ContextMenuToggle : StatefulWidget {
		// Args
		Key key;
		Key overlayKey;
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
						// Pop by key so it's a no-op if onClose already removed the menu
						Navigator::of(element).popOverlay(widget->overlayKey);
					},
					.child = Row{
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

	void ContextMenu::State::widgetUpdated() {
		selectedIndex = -1;
		if (widget->onSelectionChange) widget->onSelectionChange(-1);
	}

	void ContextMenu::State::moveSelection(int64_t delta) {
		const auto count = static_cast<int64_t>(widget->items.size());
		if (count == 0) return;

		auto next = selectedIndex;
		for (int64_t i = 0; i < count; ++i) {
			next = (next + delta + count) % count;
			if (!std::holds_alternative<Divider>(widget->items[static_cast<size_t>(next)])) break;
		}
		if (next == selectedIndex) return;

		selectedIndex = next;
		if (widget->onSelectionChange) widget->onSelectionChange(selectedIndex);
		setState();
	}

	void ContextMenu::State::activateSelected() {
		if (selectedIndex < 0 || selectedIndex >= static_cast<int64_t>(widget->items.size())) return;

		const auto &item = widget->items[static_cast<size_t>(selectedIndex)];
		std::visit(
			utils::overloaded{
				[&](const Button &button) {
					if (button.callback) button.callback();
				},
				[&](const Toggle &toggle) {
					if (toggle.callback) toggle.callback(!toggle.value);
				},
				[](const Divider &) {},
			},
			item
		);

		element->getApp()->postUpdateTasks.emplace_back([self = this->weak_from_this()]() {
			if (auto state = self.lock()) state->close();
		});
	}

	void ContextMenu::State::close() {
		if (widget->onClose) widget->onClose();
		Navigator::of(this).popOverlay(widget->overlayKey);
	}

	Child ContextMenu::State::build(const Element &) {
		Children ret;
		const auto count = static_cast<int64_t>(widget->items.size());
		for (int64_t i = 0; i < count; ++i) {
			const auto &item = widget->items[static_cast<size_t>(i)];
			const bool highlighted = (i == selectedIndex);
			std::visit(
				utils::overloaded{
					[&](const Button &button) {
						ret.emplace_back(ContextMenuButton{
							.overlayKey = widget->overlayKey,
							.label = button.text,
							.highlighted = highlighted,
							.onClick = button.callback,
							.onClose = widget->onClose,
						});
					},
					[&](const Toggle &button) {
						ret.emplace_back(ContextMenuToggle{
							.overlayKey = widget->overlayKey,
							.label = button.text,
							.onClick = button.callback,
							.currentValue = button.value,
							.onClose = widget->onClose,
						});
					},
					[&](const Divider &) {
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

		return Stack{
			.children{
				Gesture{
					.onClick = [this](const Gesture::State &) {
						close();
					},
					.onUpdate = [this](const Gesture::State &state) {
						if (state.isKeyPressedOrRepeat(GestureKey::up)) moveSelection(-1);
						if (state.isKeyPressedOrRepeat(GestureKey::down)) moveSelection(1);
						if (state.isKey(GestureKey::enter, GestureAction::press) && selectedIndex >= 0) activateSelected();
						if (state.isKeyPressedOrRepeat(GestureKey::escape)) close();
					},
					.child = Box{
						.color = Color::transparent,
					},
				},
				Offset{
					.calculateContentBounds = [this](Rect rect, const SingleChildRenderObject &renderObject) -> Rect {
						if (widget->targetKey) {
							auto *app = renderObject.getApp();

							auto target = Element::getElementForGlobalKey(widget->targetKey);
							if (!target || !target->mounted) {
								app->postRepositionTasks.emplace_back([self = this->weak_from_this()]() {
									if (auto state = self.lock()) state->close();
								});
							}
							Rect bounds = target && target->mounted
											? RenderObjectElement::getAncestorRenderObjectElement(target.get())->renderObject->getRect()
											: Rect::fromPosSize(vec2{}, vec2{});

							auto objRect = Rect::fromPosSize({bounds.left, bounds.bottom + 2.f}, renderObject.getLayoutRect().size());
							return rect.dragInside(objRect);
						}

						auto objRect = Rect::fromPosSize(widget->position, renderObject.getLayoutRect().size());
						return rect.dragInside(objRect);
					},
					.child = SlideIn{
						.child = Card{
							.widget = widget->widget.withDefaults(Args{
								.width = Size::Shrink,
								.height = Size::Wrap,
								.sizeConstraints{.minWidth = 100.f},
							}),
							.child = ScrollView{
								.widget{
									.height = Size::Wrap,
								},
								.scrollWidget{
									.padding = Padding{1.f, 4.f},
								},
								.children = std::move(ret),
							},
						},
					},
				},
			},
		};
	}
}// namespace squi
