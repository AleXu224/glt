#include "paginator.hpp"
#include "widgets/button.hpp"
#include "widgets/column.hpp"
#include "widgets/row.hpp"
#include "widgets/stack.hpp"
#include "widgets/text.hpp"
#include <algorithm>
#include <string>

namespace squi {
	core::Child Paginator::State::build(const Element &) {
		uint32_t itemCount = widget->getItemCount();
		uint32_t totalPages = (itemCount + widget->itemsPerPage - 1) / widget->itemsPerPage;
		if (totalPages == 0) totalPages = 1;

		if (current_page >= totalPages) {
			current_page = totalPages - 1;
		}

		uint32_t offset = current_page * widget->itemsPerPage;
		uint32_t count = std::min(widget->itemsPerPage, itemCount - offset);

		auto content = widget->builder(offset, count);

		auto controls = Row{
			.widget{
				.width = Size::Shrink,
				.alignment = Alignment::Center,
			},
			.crossAxisAlignment = Row::Alignment::center,
			.spacing = 10.f,
			.children = {
				Button{
					.disabled = current_page == 0,
					.onClick = [this]() {
						setState([this]() {
							if (current_page > 0) current_page--;
						});
					},
					.child = "Previous",
				},
				Text{
					.text = "Page " + std::to_string(current_page + 1) + " of " + std::to_string(totalPages),
				},
				Button{
					.disabled = current_page >= totalPages - 1,
					.onClick = [this, totalPages]() {
						setState([this, totalPages]() {
							if (current_page < totalPages - 1) current_page++;
						});
					},
					.child = "Next",
				},
			},
		};

		return Column{
			.widget = widget->widget,
			.crossAxisAlignment = Flex::Alignment::center,
			.spacing = 10.f,
			.children = {
				content,
				Stack{
					.widget{
						.height = Size::Shrink,
					},
					.children{controls}
				},
			},
		};
	}
}// namespace squi