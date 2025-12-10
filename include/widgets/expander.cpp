#include "expander.hpp"
#include "iconButton.hpp"
#include "widgets/box.hpp"
#include "widgets/column.hpp"
#include "widgets/row.hpp"
#include "widgets/text.hpp"

namespace squi {
	core::Child Expander::State::build(const Element &element) {
		return Column{
			.children{
				Box{
					.widget{
						.height = 64.f,
					},
					.color = Color::white * 0.0512f,
					.borderColor = Color::black * 0.1f,
					.borderWidth = 1.f,
					.borderRadius = BorderRadius{4.f}.withBottom(expanded ? 0.f : 4.f),
					.borderPosition = Box::BorderPosition::outset,
					.child = Row{
						.widget{
							.padding = Padding{}.withHorizontal(16.f),
						},
						.crossAxisAlignment = Row::Alignment::center,
						.children{
							widget->icon ? widget->icon : Child{},
							Column{
								.widget{
									.height = Size::Shrink,
									.margin = Margin{}.withLeft(widget->icon ? 16.f : 0.f),
								},
								.spacing = 4.f,
								.children{
									widget->title.empty()//
										? Child{}
										: Text{
											  .text = widget->title,
											  .fontSize = 14.f,
											  .color = Color::white,
										  },
									widget->subtitle.empty()//
										? Child{}
										: Text{
											  .text = widget->subtitle,
											  .fontSize = 12.f,
											  .color = Color::white * 0.7f,
										  },
								},
							},
							widget->action ? widget->action : Child{},
							widget->content//
								? IconButton{
									  .widget{
										  .margin = Margin{}.withLeft(16.f),
									  },
									  .icon = expanded ? 0xe316 : 0xe313,
									  .theme = Button::Theme::Subtle(),
									  .onClick = [this](void) {
										  setState([this]() {
											  expanded = !expanded;
										  });
									  },
								  }
								: Child{},
						},
					},
				},
				expanded//
					? Box{
						  .widget{
							  .height = Size::Shrink,
						  },
						  .color = Color::white * 0.0326f,
						  .borderColor = Color::black * 0.1f,
						  .borderWidth = BorderWidth{1.f}.withTop(0.f),
						  .borderPosition = Box::BorderPosition::outset,
						  .child = widget->content,
					  }
					: Child{},
			}
		};
	}
}// namespace squi