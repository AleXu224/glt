#include "dropdownButton.hpp"

#include <utility>

#include "align.hpp"
#include "container.hpp"
#include "fontIcon.hpp"
#include "registerEvent.hpp"
#include "row.hpp"
#include "text.hpp"
#include "window.hpp"

using namespace squi;

squi::DropdownButton::operator squi::Child() const {
	Observable<ButtonState> stateEvent;

	auto storage = std::make_shared<Storage>(items);

	auto textWidget = Text{
		.widget{
			.onInit = [textUpdater = textUpdater, stateEvent, style = style](Widget &w) {
				w.customState.add(textUpdater.observe([&w](const std::string &newText) {
					w.as<Text::Impl>().setText(newText);
				}));

				w.customState.add(stateEvent.observe([style, &w](ButtonState state) {
					auto color = [&state, &style]() {
						switch (state) {
							case ButtonState::resting:
								return style.textColor;
							case ButtonState::hovered:
								return style.textColorHover;
							case ButtonState::active:
								return style.textColorActive;
							case ButtonState::disabled:
								return style.textColorDisabled;
						}
					}();
					w.as<Text::Impl>().setColor(color);
				}));
			},
		},
		.text = text,
	};

	auto caret = FontIcon{
		.textWidget{
			.onInit = [stateEvent, style = style](Widget &w) {
				w.customState.add(stateEvent.observe([style, &w](ButtonState state) {
					auto color = [&state, &style]() {
						switch (state) {
							case ButtonState::resting:
								return style.textColor;
							case ButtonState::hovered:
								return style.textColorHover;
							case ButtonState::active:
								return style.textColorActive;
							case ButtonState::disabled:
								return style.textColorDisabled;
						}
					}();
					w.as<Text::Impl>().setColor(color);
				}));
			},
		},
		.icon = 0xE972,
		.size = 8.f,
	};

	auto button = Button{
		.widget = widget,
		.style = style,
		.disabled = disabled,
		.onClick = [storage](GestureDetector::Event event) {
			Window::of(&event.widget).addOverlay(ContextMenu{
				.position = event.widget.getPos().withYOffset(event.widget.getSize().y),
				.items = storage->items,
			});
		},
		.child = Row{
			.spacing = 6.f,
			.children{
				Container{
					.child = Align{
						.xAlign = 0.f,
						.child = textWidget,
					},
				},
				Container{
					.widget{.width = Size::Shrink},
					.child = Align{
						.child = caret,
					},
				},
			},
		},
	};
	return RegisterEvent{
		.onInit = [itemsUpdater = itemsUpdater, storage](Widget &w) {
			w.customState.add(itemsUpdater.observe([storage](std::vector<ContextMenu::Item> newItems) {
				storage->items = std::move(newItems);
			}));
		},
		.afterInit = [stateEvent](Widget &w) {
			w.customState.add(w.customState.get<Observable<ButtonState>>("stateEvent").observe([stateEvent](ButtonState state) {
				stateEvent.notify(state);
			}));
		},
		.child = button,
	};
}
