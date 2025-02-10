#include "dialog.hpp"

#include "box.hpp"
#include "column.hpp"
#include "modal.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "text.hpp"

using namespace squi;

squi::Dialog::operator Child() const {
	auto titleText = Text{
		.widget{
			.padding = Padding(24.f).withBottom(0.f),
		},
		.text = title,
		.fontSize = 20.f,
		.font = FontStore::defaultFontBold,
	};

	auto contentBox = Box{
		.color = Color::css(255, 255, 255, 0.0538f),
		.borderColor = Color::css(0, 0, 0, 0.1f),
		.borderWidth = BorderWidth::Bottom(1.f),
		.borderRadius = BorderRadius::Top(7.f),
		.borderPosition = Box::BorderPosition::outset,
		.child = Column{
			.spacing = 12.f,
			.children{
				titleText,
				ScrollableFrame{
					.scrollableWidget{
						.padding = Padding(24.f, 0.f).withBottom(24.f),
					},
					.spacing = 12.f,
					.children{content}
				},
			},
		},
	};

	auto buttonContainer = Row{
		.widget{
			.padding = 24.f,
		},
		.spacing = 8.f,
		.children = buttons,
	};

	auto dialog = Box{
		.widget{
			.width = 540.f,
			.height = Size::Shrink,
			.padding = 1.f,
		},
		.color = Color::css(32, 32, 32, 1.f),
		.borderColor = Color::css(117, 117, 117, 0.4f),
		.borderWidth = 1.f,
		.borderRadius = 8.f,
		.child = Column{
			.children{
				contentBox,
				buttonContainer,
			},
		},
	};

	return Modal{
		.widget = widget,
		.closeEvent = closeEvent,
		.child = dialog,
	};
}