#include "navigationMenu.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "column.hpp"
#include "fontIcon.hpp"
#include "observer.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "stack.hpp"
#include "text.hpp"
#include <functional>
#include <memory>

using namespace squi;

struct MenuItem {
	// Args
	Widget::Args widget;
	std::string_view name = "Menu Item";
	char32_t icon;
	std::weak_ptr<Observable<bool>> isExpandedEvent;
	std::weak_ptr<VoidObservable> selectionEvent;
	std::function<void()> onClick;
	bool isActive = false;

	struct Storage {
		// Data
		bool isActive = false;
		std::shared_ptr<Observable<bool>::Observer> expandedObserver;
		std::shared_ptr<VoidObservable::Observer> selectionObserver;
		std::shared_ptr<Observable<bool>> selectionChangeEvent = Observable<bool>::create();
	};

	operator Child() const {
		auto storage = std::make_shared<Storage>(isActive);

		if (auto event = selectionEvent.lock()) {
			storage->selectionObserver = event->observe([storage]() {
				storage->isActive = false;
			});
		}

		const auto indicatorUpdate = [storage](Widget &widget) {
			dynamic_cast<Box::Impl &>(widget).setColor(storage->isActive ? Color{0x60CDFFFF} : Color{0x00000000});
		};

		return Button{
			.widget{
				widget.withDefaultWidth(Size::Expand).withDefaultHeight(36.f).withDefaultMargin({4.f, 2.f}).withDefaultPadding(0.f),
			},
			.style = ButtonStyle::Subtle(),
			.onClick = [storage, onClick = onClick, selectionEvent = selectionEvent](auto) {
				if (storage->isActive) return;
				if (auto event = selectionEvent.lock())
					event->notify();
				storage->isActive = true;
				storage->selectionChangeEvent->notify(true);
				if (onClick) onClick();
			},
			.child = Stack{
				.children{
					Align{
						.xAlign = 0.f,
						.child = Box{
							.widget{
								.width = 3.f,
								.height = 16.f,
								.onInit = indicatorUpdate,
								.onUpdate = indicatorUpdate,
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
								.icon = icon,
								.size = 16.f,
							},
							Text{
								.widget{
									.onInit = [storage, isExpandedEvent = isExpandedEvent](Widget &widget) {
										if (auto event = isExpandedEvent.lock()) {
											storage->expandedObserver = event->observe([w = widget.weak_from_this()](bool isExpanded) {
												if (auto widget = w.lock())
													widget->setVisible(isExpanded);
											});
										}
									},
								},
								.text = name,
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
		.widget{
			.width = 320.f,
			.onInit = [storage](Widget &w) {
				storage->expandedObserver = storage->isExpandedEvent->observe([&w](bool isExpanded) {
					w.setWidth(isExpanded ? 320.f : 48.f);
				});
			},
		},
		.children{
			backAction ? Button{
							 .widget{
								 .width = 40.f,
								 .height = 36.f,
								 .margin = Margin{4.f, 2.f},
								 .padding = Padding{12.f, 10.f},
							 },
							 .style = ButtonStyle::Subtle(),
							 .onClick = [backAction = backAction](auto) {
								 backAction();
							 },
							 .child = Align{
								 .child = FontIcon{
									 .icon = 0xE830,
									 .size = 12.f,
								 },
							 },
						 }
					   : Child{},
			Button{
				.widget{
					.width = 40.f,
					.height = 36.f,
					.margin = Margin{4.f, 2.f},
					.padding = Padding{12.f, 10.f},
				},
				.style = ButtonStyle::Subtle(),
				.onClick = [storage](auto) {
					storage->isExpandedEvent->notify(storage->isExpanded = !storage->isExpanded);
				},
				.child = FontIcon{
					.icon = 0xE700,
					.size = 16.f,
				},
			},
			ScrollableFrame{
				.children = std::invoke([&]() -> Children {
					Children children{};
					bool first = true;
					for (auto &item: items) {
						children.emplace_back(MenuItem{
							.name = item.name,
							.icon = item.icon,
							.isExpandedEvent = storage->isExpandedEvent,
							.selectionEvent = storage->selectionEvent,
							.onClick = item.onClick,
							.isActive = first,
						});
						first = false;
					}
					return children;
				}),
			},
		},
	};
}