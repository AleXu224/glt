#include "navigationMenu.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "column.hpp"
#include "fontIcon.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "stack.hpp"
#include "text.hpp"

using namespace squi;

struct MenuItem {
	// Args
	Widget::Args widget;

	struct Storage {
		// Data
		bool isActive = true;
	};

	operator Child() const {
		auto storage = std::make_shared<Storage>();

		return Button{
			.widget{
				widget
					.withDefaultWidth(Size::Expand)
					.withDefaultHeight(36.f)
					.withDefaultMargin({4.f, 2.f})
					.withDefaultPadding(0.f),
			},
			.style = ButtonStyle::Subtle(),
			.onClick = [storage](auto) {
				storage->isActive = !storage->isActive;
			},
			.child = Stack{
				.children{
					Align{
						.xAlign = 0.f,
						.child = Box{
							.widget{
								.width = 3.f,
								.height = 16.f,
								.onUpdate = [storage](Widget &widget) {
									dynamic_cast<Box::Impl &>(widget).setColor(storage->isActive ? Color{0x60CDFFFF} : Color{0x00000000});
								},
							},
							.borderRadius = 1.5f,
						},
					},
					Row{
						.widget{
							.margin = Margin{12.f, 0.f},
						},
						.alignment = Row::Alignment::center,
						.spacing = 16.f,
						.children{
							FontIcon{
								.icon = 0xEA3A,
								.size = 16.f,
							},
							Text{
								.text = "Menu Item",
							},
						},
					},
				},
			},
		};
	}
};

NavigationMenu::operator Child() const {
	auto storage = std::make_shared<Storage>();

	return Column{
		.widget{widget.withDefaultWidth(320.f).withDefaultHeight(Size::Expand)},
		.children{
			Button{
				.widget{
					.width = 40.f,
					.height = 36.f,
					.margin = Margin{4.f, 2.f},
					.padding = Padding{12.f, 10.f},
				},
				.style = ButtonStyle::Subtle(),
				.child = FontIcon{
					.icon = 0xE700,
					.size = 16.f,
				},
			},
			ScrollableFrame{
				.children{
					MenuItem{},
				},
			},
		},
	};
}