#include "widgets/liteFilter.hpp"

#include "button.hpp"
#include "observer.hpp"
#include "row.hpp"
#include "wrapper.hpp"


using namespace squi;

struct LiteFilterButton {
	// Args
	squi::Widget::Args widget{};
	std::string_view name;
	Observable<bool> selectedEvent;
	std::function<void(Widget &)> clickedEvent;

	operator squi::Child() const {
		static ButtonStyle activeStyle = []() {
			auto style = ButtonStyle::Accent();
			style.borderRadius = 16.f;
			return style;
		}();
		static ButtonStyle inActiveStyle = []() {
			auto style = ButtonStyle::Standard();
			style.borderRadius = 16.f;
			return style;
		}();

		return Button{
			.widget{
				.onInit = [selectedEvent = selectedEvent](Widget &w) {
					w.customState.add("active", false);
					observe(w, selectedEvent, [&w](bool selected) {
						w.customState.get<bool>("active") = selected;
						if (selected) {
							Button::State::style.of(w) = activeStyle;
						} else {
							Button::State::style.of(w) = inActiveStyle;
						}
					});
				},
			},
			.text = name,
			.style = inActiveStyle,
			.onClick = [clickedEvent = clickedEvent](GestureDetector::Event event) {
				if (clickedEvent) clickedEvent(event.widget);
			},
		};
	}
};

squi::LiteFilter::operator squi::Child() const {
	auto storage = std::make_shared<Storage>(items);
	Observable<bool> allSelectedEvent;

	return Wrapper{
		.afterInit = [allSelectedEvent](Widget &) {
			allSelectedEvent.notify(true);
		},
		.child = Row{
			.widget = widget,
			.spacing = 4.f,
			.children = [&]() {
				Children ret{LiteFilterButton{
					.name = "All",
					.selectedEvent = allSelectedEvent,
					.clickedEvent = [storage, multiSelect = multiSelect, allSelectedEvent](Widget &) {
						if (!storage->allSelected) {
							allSelectedEvent.notify(true);
							storage->allSelected = true;
							storage->selectedItemsCount = 0;

							for (const auto &event: storage->events) {
								event.notify(false);
							}
							for (const auto &item: storage->items) {
								if (item.onUpdate) item.onUpdate(true);
							}
						} else if (multiSelect) {
							storage->allSelected = false;
							allSelectedEvent.notify(false);

							for (const auto &event: storage->events) {
								event.notify(true);
							}
							storage->selectedItemsCount = storage->events.size();
						}
					},
				}};

				for (const auto &item: items) {
					Observable<bool> selectedEvent;
					storage->events.emplace_back(selectedEvent);

					ret.emplace_back(LiteFilterButton{
						.name = item.name,
						.selectedEvent = selectedEvent,
						.clickedEvent = [selectedEvent, storage, allSelectedEvent, multiSelect = multiSelect, onUpdate = item.onUpdate](Widget &w) {
							if (w.customState.get<bool>("active")) {
								storage->selectedItemsCount--;
								if (storage->selectedItemsCount == 0) {
									storage->allSelected = true;
									allSelectedEvent.notify(true);
									for (const auto &item: storage->items) {
										if (item.onUpdate) item.onUpdate(true);
									}
								}

								selectedEvent.notify(false);
								if (!storage->allSelected && onUpdate) onUpdate(false);
							} else {
								if (!multiSelect) {
									for (const auto &event: storage->events) {
										event.notify(false);
									}
									storage->selectedItemsCount = 0;
								}

								storage->selectedItemsCount++;
								if (storage->allSelected) {
									storage->allSelected = false;
									allSelectedEvent.notify(false);
									for (const auto &item: storage->items) {
										if (item.onUpdate) item.onUpdate(false);
									}
								}

								selectedEvent.notify(true);
								if (onUpdate) onUpdate(true);
							}
						},
					});
				}

				return ret;
			}(),
		},
	};
}
