#include "sideNav.hpp"
#include "theme.hpp"
#include "utils.hpp"
#include "widgets/animatedBox.hpp"
#include "widgets/animatedText.hpp"
#include "widgets/column.hpp"
#include "widgets/container.hpp"
#include "widgets/fontIcon.hpp"
#include "widgets/iconButton.hpp"
#include "widgets/row.hpp"
#include "widgets/scrollable.hpp"
#include "widgets/stack.hpp"


namespace squi {
	struct SideNavButton : StatefulWidget {
		// Args
		Key key;
		const SideNav::Page &page;
		bool isSelected{false};
		bool expanded{true};
		std::function<void()> onClick;

		struct State : WidgetState<SideNavButton> {
			Button::ButtonStatus buttonStatus = Button::ButtonStatus::resting;

			Child build(const Element &element) override {
				auto theme = Button::Theme::Subtle();
				if (widget->isSelected) {
					theme.resting.backgroundColor = theme.hovered.backgroundColor;
				}

				return Container{
					.widget{
						.width = Size::Wrap,
						.height = Size::Wrap,
					},
					.child = Button{
						.widget{
							.width = Size::Expand,
							.height = 34.f,
							.padding = 0.f,
						},
						.theme = theme,
						.onStatusChange = [this](Button::ButtonStatus status) {
							setState([this, status]() {
								buttonStatus = status;
							});
						},
						.onClick = [this]() {
							if (widget->onClick) widget->onClick();
						},
						.child = Stack{
							.children{
								AnimatedBox{
									.widget{
										.width = 3.f,
										.height = 16.f,
										.alignment = Alignment::CenterLeft,
									},
									.color = widget->isSelected ? ThemeManager::getTheme().accent : Color::transparent,
									.borderRadius = 2.f,
								},
								Scrollable{
									.widget{
										.padding = Padding{12.f, 0.f},
									},
									.alignment = Flex::Alignment::center,
									.direction = Axis::Horizontal,
									.spacing = 16.f,
									.children = {
										std::visit(//
											utils::overloaded{
												[](int32_t iconCode) -> Child {
													return FontIcon{
														.icon = iconCode,
													};
												},
												[](const Child &iconChild) -> Child {
													return iconChild;
												},
											},
											widget->page.icon
										),
										widget->expanded//
											? AnimatedText{
												  .text = widget->page.name,
												  .color = theme.fromStatus(buttonStatus).textColor,
											  }
											: Child{},
									},
								},
							},
						},
					},
				};
			}
		};
	};

	core::Child SideNav::State::build(const Element &element) {
		return Row{
			.widget = widget->widget,
			.children{
				Column{
					.widget{
						.width = navWidth,
						.padding = 4.f,
					},
					.spacing = 4.f,
					.children{
						widget->backAction//
							? IconButton{
								  .icon = 0xe5c4,
								  .theme = Button::Theme::Subtle(),
								  .onClick = widget->backAction,
							  }
							: Child{},
						IconButton{
							.icon = 0xe5d2,
							.theme = Button::Theme::Subtle(),
							.onClick = [this]() {
								setState([this]() {
									expanded = !expanded;
									navWidth = expanded ? 320.f : 48.f;
								});
							},
						},
						Column{
							.spacing = 4.f,
							.children = [this]() {
								Children ret;

								for (const auto &[index, page]: widget->pages | std::views::enumerate) {
									ret.emplace_back(SideNavButton{
										.page = page,
										.isSelected = index == currentPageIndex,
										.expanded = expanded,
										.onClick = [this, index]() {
											setState([&]() {
												currentPageIndex = index;
											});
										},
									});
								}

								return ret;
							}(),
						},
					},
				},
				widget->pages.empty()//
					? Child{}
					: widget->pages.at(currentPageIndex).content,
			},
		};
	}
}// namespace squi