#include "dropdownButton.hpp"

#include <utility>

#include "align.hpp"
#include "container.hpp"
#include "fontIcon.hpp"
#include "row.hpp"
#include "text.hpp"
#include "window.hpp"
#include "wrapper.hpp"


using namespace squi;

squi::DropdownButton::operator squi::Child() const {
	Observable<Color> colorEvent;

	auto storage = std::make_shared<Storage>(items);

	auto textWidget = Text{
		.widget{
			.onInit = [textUpdater = textUpdater, colorEvent](Widget &w) {
				w.customState.add(textUpdater.observe([&w](const std::string &newText) {
					w.as<Text::Impl>().setText(newText);
				}));

				w.customState.add(colorEvent.observe([&w](Color color) {
					w.as<Text::Impl>().setColor(color);
				}));
			},
		},
		.text = text,
	};

	auto caret = FontIcon{
		.textWidget{
			.onInit = [colorEvent](Widget &w) {
				w.customState.add(colorEvent.observe([&w](Color color) {
					w.as<Text::Impl>().setColor(color);
				}));
			},
		},
		.icon = 0xe313,
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
	return Wrapper{
		.onInit = [itemsUpdater = itemsUpdater, storage](Widget &w) {
			observe(w, itemsUpdater, [storage](std::vector<ContextMenu::Item> newItems) {
				storage->items = std::move(newItems);
			});
		},
		.afterInit = [colorEvent](Widget &w) {
			observe(w, Button::State::stateEvent.of(w), [colorEvent, &w](ButtonState state) {
				auto style = *Button::State::style.of(w);
				switch (state) {
					case ButtonState::resting:
						colorEvent.notify(style.textColor);
						break;
					case ButtonState::hovered:
						colorEvent.notify(style.textColorHover);
						break;
					case ButtonState::active:
						colorEvent.notify(style.textColorActive);
						break;
					case ButtonState::disabled:
						colorEvent.notify(style.textColorDisabled);
						break;
				}
			});
			colorEvent.notify(Button::State::style.of(w)->textColor);
		},
		.child = button,
	};
}
