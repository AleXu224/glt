#include "navigationView.hpp"

#include "navigationMenu.hpp"
#include "row.hpp"
#include "stack.hpp"
#include <functional>
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
								if (storage->currentPage) storage->currentPage->flags.visible = false;
								w->flags.visible = true;
								storage->currentPage = w.get();
							},
						});
					}
					return items;
				}),
			},
			Stack{
				.widget{
					.onInit = [pages = pages, storage](Widget &w) {
						for (const auto &page: pages) {
							page.content->flags.visible = false;
							w.addChild(page.content);
						}
						if (!pages.empty()) {
							pages.front().content->flags.visible = true;
							storage->currentPage = pages.front().content.get();
						}
					},
				},
			},
		},
	};
}
