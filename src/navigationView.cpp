#include "navigationView.hpp"

#include "container.hpp"
#include "navigationMenu.hpp"
#include "row.hpp"
#include <functional>
#include <utility>
#include <vector>


using namespace squi;

NavigationView::operator Child() const {
	auto storage = std::make_shared<Storage>();

	return Row{
		.widget = widget,
		.children{
			NavigationMenu{
				.expanded = expanded,
				.backAction = backAction,
				.items = std::invoke([&]() -> std::vector<NavigationMenu::Item> {
					std::vector<NavigationMenu::Item> items{};
					items.reserve(pages.size());
					for (const auto &page: pages) {
						items.push_back(NavigationMenu::Item{
							.name = page.name,
							.icon = page.icon,
							.onClick = [w = page.content, storage]() {
								storage->contentChangeEvent.notify(w);
							},
						});
					}
					return items;
				}),
			},
			Container{
				.widget{
					.onInit = [storage](Widget &widget) {
						storage->contentChangeObserver = storage->contentChangeEvent.observe([w = widget.weak_from_this()](Child content) {
							if (auto widget = w.lock())
								widget->setChildren({std::move(content)});
						});
					},
				},
				.child = pages.empty() ? Child{} : pages.front().content,
			},
		},
	};
}
