#pragma once

#include "stateInfo.hpp"
#include "widget.hpp"

namespace squi {
	struct Paginator {
		// Args
		squi::Widget::Args widget{};
        VoidObservable refreshItemsEvent;
		uint32_t itemsPerPage = 50;
		std::function<uint32_t()> getItemCount;
		std::function<Child(uint32_t offset, uint32_t count)> builder;

		struct State {
			const static inline StateInfo<VoidObservable> refreshItems{.name = "refreshItems"};
		};

		operator squi::Child() const;
	};
}// namespace squi