#include "dropdownButton.hpp"

#include "container.hpp"
#include "fontIcon.hpp"
#include "row.hpp"
#include "window.hpp"

using namespace squi;

squi::DropdownButton::operator squi::Child() const {
	auto storage = std::make_shared<Storage>();

	return Button{
		.widget = widget,
		.style = style,
		.onClick = [items = items](GestureDetector::Event event) {
			Window::of(&event.widget).addOverlay(ContextMenu{
				.position = event.widget.getPos().withYOffset(event.widget.getSize().y),
				.items = items,
			});
		},
		.child = Row{
			.spacing = 4.f,
			.children{
				Container{
					.child = Align{
						.xAlign = 0.f,
						.child = Text{
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
	};
}
