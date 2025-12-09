#include "topNav.hpp"
#include "widgets/box.hpp"
#include "widgets/column.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/row.hpp"
#include "widgets/stack.hpp"
#include "widgets/text.hpp"

#include "theme.hpp"

namespace squi {
	struct TopNavButton : StatelessWidget {
		// Args
		Key key;
		const TopNav::Page &page;
		bool isSelected{false};
		std::function<void()> onClick;

		Child build(const Element &) const {
			return Gesture{
				.onClick = [this](auto) {
					if (this->onClick) this->onClick();
				},
				.child = Stack{
					.widget{
						.width = Size::Shrink,
					},
					.children{
						Text{
							.widget{
								.alignment = Alignment::Center,
								.margin = Margin{12.f, 0.f},
							},
							.text = page.name,
						},
						isSelected//
							? Box{
								  .widget{
									  .width = 16.f,
									  .height = 3.f,
									  .alignment = Alignment::BottomCenter,
								  },
								  .color = ThemeManager::getTheme().accent,
								  .borderRadius = 2.f,
							  }
							: Child{},
					},
				},
			};
		}
	};

	squi::core::Children squi::TopNav::State::getButtons() {
		Children ret;
		ret.reserve(widget->pages.size());

		for (const auto &[index, page]: widget->pages | std::views::enumerate) {
			ret.emplace_back(TopNavButton{
				.page = page,
				.isSelected = index == currentPageIndex,
				.onClick = [this, index]() {
					setState([&]() {
						currentPageIndex = index;
					});
				},
			});
		}

		return ret;
	}

	squi::core::Child squi::TopNav::State::build(const Element &element) {
		return Gesture{
			.onUpdate = [&](const Gesture::State &state) {
				if (state.isKeyPressedOrRepeat(GestureKey::tab, GestureMod::control)) {
					setState([&]() {
						currentPageIndex = (currentPageIndex + 1) % widget->pages.size();
					});
				} else if (state.isKeyPressedOrRepeat(GestureKey::tab, GestureMod::control | GestureMod::shift)) {
					setState([&]() {
						currentPageIndex = (currentPageIndex - 1 + widget->pages.size()) % widget->pages.size();
					});
				}

				for (const auto &[index, page]: widget->pages | std::views::enumerate | std::views::take(9)) {
					if (state.isKeyPressedOrRepeat(GestureKey::n0 + index + 1, GestureMod::control)) {
						setState([&]() {
							currentPageIndex = index;
						});
					}
				}
			},
			.child = Column{
				.widget = widget->widget,
				.children{
					Row{
						.widget{
							.height = 40.f,
							.margin = 4.f,
						},
						.children = getButtons(),
					},
					Box{
						.widget{
							.padding = Padding{}.withTop(1.f),
						},
						.color = Color::css(58, 58, 58, 0.3f),
						.borderColor = Color::black * 0.1f,
						.borderWidth = BorderWidth::Top(1.f),
						.borderPosition = Box::BorderPosition::outset,
						.child = widget->pages.at(currentPageIndex).content,
					},
				},
			},
		};
	}
}// namespace squi
