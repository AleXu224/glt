#include "liteFilter.hpp"
#include "widgets/button.hpp"
#include "widgets/row.hpp"

namespace squi {
	struct LiteFilterButton : StatelessWidget {
		// Args
		Key key;
		std::string name;
		bool selected = false;
		std::function<void()> onClick;

		[[nodiscard]] Child build(const Element &) const {
			auto theme = selected ? Button::Theme::Accent() : Button::Theme::Standard();
			theme.resting.borderRadius = 16.f;
			theme.hovered.borderRadius = 16.f;
			theme.active.borderRadius = 16.f;
			theme.disabled.borderRadius = 16.f;

			return Button{
				.theme = theme,
				.onClick = onClick,
				.child = name,
			};
		}
	};

	[[nodiscard]] core::Children LiteFilter::State::createButtons() {
		core::Children buttons;
		buttons.emplace_back(LiteFilterButton{
			.name = "All",
			.selected = selectedIndices.size() == widget->items.size(),
			.onClick = [this]() {
				setState([&]() {
					for (size_t i = 0; i < widget->items.size(); ++i) {
						if (!selectedIndices.contains(i) && widget->items[i].onUpdate) {
							widget->items[i].onUpdate(true);
							selectedIndices.insert(i);
						}
					}
				});
			},
		});
		for (size_t i = 0; i < widget->items.size(); ++i) {
			bool isSelected = selectedIndices.contains(i);
			bool allSelected = selectedIndices.size() == widget->items.size();
			buttons.push_back(LiteFilterButton{
				.name = widget->items[i].name,
				.selected = isSelected && !allSelected,
				.onClick = [this, i, isSelected, allSelected]() {
					if (allSelected) {
						// Deselect all except this one
						setState([&]() {
							selectedIndices.clear();
							selectedIndices.insert(i);
						});
						for (size_t j = 0; j < widget->items.size(); ++j) {
							if (j != i && widget->items[j].onUpdate) {
								widget->items[j].onUpdate(false);
							}
						}
					} else if (isSelected) {
						// Deselect this one
						setState([&]() {
							selectedIndices.erase(i);
						});
						if (widget->items[i].onUpdate) {
							widget->items[i].onUpdate(false);
						}
					} else {
						// Select this one
						setState([&]() {
							selectedIndices.insert(i);
						});
						if (widget->items[i].onUpdate) {
							widget->items[i].onUpdate(true);
						}
					}
				},
			});
		}
		return buttons;
	}

	core::Child LiteFilter::State::build(const Element &) {
		return Row{
			.spacing = 4.f,
			.children = createButtons(),
		};
	}
}// namespace squi