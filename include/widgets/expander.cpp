#include "expander.hpp"
#include "iconButton.hpp"
#include "utils.hpp"
#include "widgets/box.hpp"
#include "widgets/column.hpp"
#include "widgets/row.hpp"
#include "widgets/slideIn.hpp"
#include "widgets/text.hpp"


namespace squi {
	core::Child Expander::State::build(const Element &) {
		return Column{
			.widget = widget->widget,
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
									std::visit(
										utils::overloaded{
											[](const std::string &val) -> Child {
												if (val.empty()) return Child{};
												return Text{
													.text = val,
													.fontSize = 14.f,
													.color = Color::white,
												};
											},
											[](const Child &val) -> Child {
												return val;
											},
										},
										widget->title
									),
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
							widget->content && !widget->alwaysExpanded//
								? IconButton{
									  .widget{
										  .margin = Margin{}.withLeft(16.f),
									  },
									  .icon = expanded ? static_cast<char32_t>(0xe316) : static_cast<char32_t>(0xe313),
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
					? SlideIn{
						  .direction = Direction::top,
						  .child = Box{
							  .widget{
								  .height = Size::Shrink,
							  },
							  .color = Color::white * 0.0326f,
							  .borderColor = Color::black * 0.1f,
							  .borderWidth = BorderWidth{1.f}.withTop(0.f),
							  .borderPosition = Box::BorderPosition::outset,
							  .child = widget->content,
						  },
					  }
					: Child{},
			}
		};
	}
}// namespace squi