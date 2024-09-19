#include "dropdownButton.hpp"

#include <utility>

#include "container.hpp"
#include "fontIcon.hpp"
#include "registerEvent.hpp"
#include "row.hpp"
#include "window.hpp"

using namespace squi;

squi::DropdownButton::operator squi::Child() const {
	auto storage = std::make_shared<Storage>(items);

	return RegisterEvent{
		.onInit = [itemsUpdater = itemsUpdater, storage](Widget &w) {
			w.customState.add(itemsUpdater.observe([storage](std::vector<ContextMenu::Item> newItems) {
				storage->items = std::move(newItems);
			}));
		},
		.child = Button{
			.widget = widget,
			.style = style,
			.onClick = [storage](GestureDetector::Event event) {
				Window::of(&event.widget).addOverlay(ContextMenu{
					.position = event.widget.getPos().withYOffset(event.widget.getSize().y),
					.items = storage->items,
				});
			},
			.child = Row{
				.spacing = 4.f,
				.children{
					Container{
						.child = Align{
							.xAlign = 0.f,
							.child = Text{
								.widget{
									.onInit = [textUpdater = textUpdater](Widget &w) {
										w.customState.add(textUpdater.observe([&w](const std::string &newText) {
											w.as<Text::Impl>().setText(newText);
										}));
									},
								},
								.text = text,
							},
						},
					},
					Container{
						.widget{.width = Size::Shrink},
						.child = Align{
							.child = FontIcon{
								.icon = 0xE972,
							},
						},
					},
				},
			},
		},
	};
}
