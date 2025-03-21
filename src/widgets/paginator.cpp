#include "widgets/paginator.hpp"
#include "button.hpp"
#include "column.hpp"
#include "rebuilder.hpp"
#include "row.hpp"
#include "text.hpp"

using namespace squi;

namespace {
	struct PaginatorState {
		uint32_t currentPage;
		uint32_t itemCount;
		uint32_t pageSize;

		uint32_t getPageCount() {
			auto minPage = itemCount / pageSize;
			if (itemCount % pageSize != 0) minPage++;
			return std::max(minPage, static_cast<uint32_t>(1));
		}
	};
	struct PaginatorButtons {
		// Args
		squi::Widget::Args widget{};
		std::shared_ptr<PaginatorState> state;
		VoidObservable pageChangeEvent;

		operator squi::Child() const {
			return Row{
				.widget{
					.width = Size::Shrink,
					.height = Size::Shrink,
				},
				.alignment = squi::Row::Alignment::center,
				.spacing = 4.f,
				.children{
					Button{
						.text = "Previous",
						.style = ButtonStyle::Standard(),
						.disabled = state->currentPage <= 0,
						.onClick = [state = state, pageChangeEvent = pageChangeEvent](GestureDetector::Event) {
							state->currentPage--;
							pageChangeEvent.notify();
						},
					},
					Text{
						.text = std::format("{}/{}", state->currentPage + 1, state->getPageCount()),
					},
					Button{
						.text = "Next",
						.style = ButtonStyle::Standard(),
						.disabled = (state->currentPage + 1) >= state->getPageCount(),
						.onClick = [state = state, pageChangeEvent = pageChangeEvent](GestureDetector::Event) {
							state->currentPage++;
							pageChangeEvent.notify();
						},
					},
				},
			};
		}
	};
}// namespace

squi::Paginator::operator squi::Child() const {
	VoidObservable pageChangeEvent;
	auto state = std::make_shared<PaginatorState>();
	state->pageSize = itemsPerPage;
	state->itemCount = getItemCount();

	Child ret = Rebuilder{
		.rebuildEvent = refreshItemsEvent,
		.buildFunc = [pageChangeEvent, widget = widget, builder = builder, getItemCount = getItemCount, state]() {
			state->itemCount = getItemCount();
			state->currentPage = std::min(state->currentPage, state->getPageCount() - 1);
			return Rebuilder{
				.rebuildEvent = pageChangeEvent,
				.buildFunc = [widget = widget, builder, state, pageChangeEvent]() {
					return Column{
						.widget = widget,
						.alignment = Column::Alignment::center,
						.spacing = 4.f,
						.children{
							state->itemCount != 0 ? PaginatorButtons{
														.state = state,
														.pageChangeEvent = pageChangeEvent,
													}
												  : Child{},
							builder(state->currentPage * state->pageSize, std::min(state->pageSize, state->itemCount - (state->currentPage * state->pageSize))),
							state->itemCount != 0 ? PaginatorButtons{
														.state = state,
														.pageChangeEvent = pageChangeEvent,
													}
												  : Child{},
						},
					};
				},
			};
		},
	};

	State::refreshItems.bind(ret, refreshItemsEvent);

	return ret;
}
