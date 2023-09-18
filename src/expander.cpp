#include "expander.hpp"
#include "box.hpp"
#include "column.hpp"
#include "fontIcon.hpp"
#include "row.hpp"
#include "text.hpp"

using namespace squi;

Expander::operator Child() const {
	auto storage = std::make_shared<Storage>();

	return Box{
		.widget = widget.withDefaultHeight(64.f),
		.color = Color::HEX(0xFFFFFF0D),
		.borderColor = Color::HEX(0x0000001A),
		.borderWidth = 1.f,
		.borderRadius = 4.f,
		.borderPosition = Box::BorderPosition::outset,
		.child = Row{
			.widget{
				.padding = Padding{16.f, 14.f},
			},
			.alignment = Row::Alignment::center,
			.spacing = 16.f,
			.children{
				icon.has_value() ? FontIcon{
									   .icon = icon.value(),
									   .size = 16.f,
								   }
								 : Child{},
				Column{
					.widget{
						.height = Size::Shrink,
					},
					.children{
						Text{
							.text = heading,
							.fontSize = 14.f,
						},
						caption.has_value() ? Text{
												  .text = caption.value(),
												  .fontSize = 12.f,
												  .color = Color::HEX(0xFFFFFFC8),
											  }
											: Child{},
					},
				},
				child,
			},
		},
	};
}
