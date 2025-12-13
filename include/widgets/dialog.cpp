#include "dialog.hpp"
#include "modal.hpp"
#include "widgets/box.hpp"
#include "widgets/column.hpp"
#include "widgets/row.hpp"
#include "widgets/scrollview.hpp"
#include "widgets/text.hpp"

namespace squi {
	core::Child Dialog::State::build(const Element &) {
		Child titleText = Text{
			.widget{
				.padding = Padding(24.f).withBottom(0.f),
			},
			.text = widget->title,
			.fontSize = 20.f,
			.font = FontStore::defaultFontBold,
		};

		auto contentBox = Box{
			.widget{.height = Size::Wrap},
			.color = Color::css(255, 255, 255, 0.0538f),
			.borderColor = Color::css(0, 0, 0, 0.1f),
			.borderWidth = BorderWidth::Bottom(1.f),
			.borderRadius = BorderRadius::Top(7.f),
			.borderPosition = Box::BorderPosition::outset,
			.child = Column{
				.widget{.height = Size::Wrap},
				.spacing = 12.f,
				.children{
					titleText,
					ScrollView{
						.widget{.height = Size::Wrap},
						.scrollWidget{
							.padding = Padding(24.f, 0.f).withBottom(24.f),
						},
						.spacing = 12.f,
						.children{widget->content}
					},
				},
			},
		};

		Child buttonContainer = Row{
			.widget{
				.height = Size::Shrink,
				.padding = 24.f,
			},
			.spacing = 8.f,
			.children = widget->buttons,
		};

		auto dialog = Box{
			.widget{
				.width = widget->width,
				.height = Size::Wrap,
				.padding = 1.f,
			},
			.color = Color::css(32, 32, 32, 1.f),
			.borderColor = Color::css(117, 117, 117, 0.4f),
			.borderWidth = 1.f,
			.borderRadius = 8.f,
			.child = Column{
				.widget{.height = Size::Wrap},
				.children{
					contentBox,
					buttonContainer,
				},
			},
		};

		return Modal{
			.widget = widget->widget,
			.closeEvent = widget->closeEvent,
			.child = dialog,
		};
	}
}// namespace squi