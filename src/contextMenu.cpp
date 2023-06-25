#include "contextMenu.hpp"
#include "align.hpp"
#include "box.hpp"
#include "child.hpp"
#include "column.hpp"
#include "container.hpp"
#include "fontIcon.hpp"
#include "gestureDetector.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "stack.hpp"
#include "text.hpp"
#include <algorithm>
#include <any>
#include <memory>
#include <numeric>
#include <optional>
#include <variant>
#include <vector>

using namespace squi;

struct ContextMenuButton {
	// Args
	const ContextMenu::Item &item;
	ChildRef root;
	ChildRef menu;
	std::optional<ChildRef> menuToLock{};
	// In the case of a context menu that has one or more items that have an icon
	// then all text will need to have the same horizontal position.
	bool shouldReserveSpace = false;
	bool shouldReserveEndSpace = false;

	struct Storage {
		// Data
		std::optional<ChildRef> menuToLock{};
		ChildRef menu;
		std::variant<std::function<void()>, ContextMenu::Item::Submenu, ContextMenu::Item::Toggle, ContextMenu::Item::Divider> content;
		bool hovered = false;
		bool submenuHovered = false;
		bool submenuOpened = false;
		bool stateChanged = false;
		ChildRef submenu{};
	};

	operator Child() const;
};

struct ContextMenuFrame {
	// Args
	vec2 position;
	const std::vector<ContextMenu::Item> &items;
	ChildRef root;
	std::optional<ChildRef> menuToLock{};

	operator Child() const;
};

ContextMenuButton::operator Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.menuToLock = menuToLock,
		.menu = menu,
		.content = item.content,
	});

	return GestureDetector{
		.onEnter = [storage = storage](auto &, auto &) {
			storage->hovered = true;
			storage->stateChanged = true; },
		.onLeave = [storage = storage](auto &, auto &) {
			storage->hovered = false;
			storage->stateChanged = true; },
		.onClick = [storage = storage, root = root](Widget &, GestureDetector::Storage &) {
			if (Child rootWidget = root.lock()) {
				switch(storage->content.index()) {
					case 0: {
						rootWidget->deleteLater();
						auto &f = std::get<0>(storage->content);
						if (f) f();
						break;
					}
					case 2: {
						rootWidget->deleteLater();
						auto &t = std::get<2>(storage->content);
						t.value = !t.value;
						if (t.callback) t.callback(t.value);
					}
				} 
			}
		},
		.onUpdate = [storage = storage, root = root](Widget &w, auto) {
			Child rootWidget = root.lock();
			if (!rootWidget) return;
			auto &rootState = std::any_cast<ContextMenu::Storage&>(rootWidget->state.properties.at("state"));
			if (storage->content.index() == 1 && storage->submenuOpened) {
				if (Child menuWidget = storage->menu.lock()) {
					const auto &menuLocked = std::any_cast<bool&>(menuWidget->state.properties.at("locked"));
					if (menuLocked) return;
				}
			}
			if (!storage->stateChanged) return;
			if (!storage->hovered && !storage->submenuHovered) {
				reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0x00000000));
				if (storage->content.index() == 1 && storage->submenuOpened) {
					ContextMenu::Storage::removeMenu(storage->submenu);
					storage->submenuOpened = false;
					if (storage->menuToLock.has_value()) {
						if (Child menuToLockWidget = storage->menuToLock.value().lock()) {
							auto &locked = std::any_cast<bool&>(menuToLockWidget->state.properties.at("locked"));
							locked = false;
						}
					}
				}
			} else {
				reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0xFFFFFF0F));
				if (storage->content.index() == 1 && !storage->submenuOpened) {
					storage->submenuOpened = true;
					if (storage->menuToLock.has_value()) {
						if (Child menuToLockWidget = storage->menuToLock.value().lock()) {
							auto &locked = std::any_cast<bool&>(menuToLockWidget->state.properties.at("locked"));
							locked = true;
						}
					}
					storage->submenuHovered = false;
					const auto &rect = w.getRect();
					storage->submenu = rootState.addMenu(GestureDetector{
						.onEnter = [storage = storage](Widget &w, auto &s) {
							storage->submenuHovered = true;
							storage->stateChanged = true;
						},
						.onLeave = [storage = storage](auto &w, auto &s) {
							storage->submenuHovered = false;
							storage->stateChanged = true;
						},
						.child{
							ContextMenuFrame{
								.position = vec2{rect.right, rect.top},
								.items = std::get<1>(storage->content).items,
								.root = root,
								.menuToLock = storage->menu,
							},
						},
					});
				}
			} },
		.child{
			Box{
				.widget{
					.height = 28.f,
					.margin{5.f, 2.f},
					.padding{4.f, 0.f},
				},
				.color{Color::HEX(0)},
				.borderRadius = 4.f,
				.child{
					Row{
						.alignment = Row::Alignment::center,
						.children{
							[&]() -> Child {
								switch (item.content.index()) {
									case 2: {// Toggle
										return FontIcon{
											.margin{0.f, 14.f, 0.f, 9.f},
											.icon{"\uF78C"},
											.font{R"(C:\Windows\Fonts\segmdl2.ttf)"},
											.size = 12.f,
										};
									}
									default: {
										if (shouldReserveSpace) {
											return Container{
												.widget{
													.width = 14.f + 9.f + 12.f,
													.height = 12.f,
												},
											};
										} else {
											return {};
										}
									}
								}
							}(),
							Text{
								.text{item.text},
							},
							Container{},// Expands by default to fill the row
							[&]() -> Child {
								switch (item.content.index()) {
									case 1: {// Toggle
										return FontIcon{
											.margin{7.f, 0.f},
											.icon{"\uE974"},
											.font{R"(C:\Windows\Fonts\segmdl2.ttf)"},
											.size = 12.f,
										};
									}
									default: {
										if (shouldReserveEndSpace) {
											return Container{
												.widget{
													.width = 7.f * 2.f + 12.f,
													.height = 12.f,
												},
											};
										} else {
											return {};
										}
									}
								}
							}(),
						},
					},
				},
			},
		},
	};
}

ContextMenuFrame::operator Child() const {
	const bool shouldReserveSpace = std::ranges::any_of(items, [](const auto &item) { return item.content.index() == 2; });
	const bool shouldReserveEndSpace = std::ranges::any_of(items, [](const auto &item) { return item.content.index() == 1; });
	Child box = Box{
		.widget{
			.width = Size::Shrink,
			.height = Size::Shrink,
			.margin{vec2{position.x, position.y}, vec2{0}},
			.padding{1.f, 4.f},
			.onInit = [](Widget &w) {
				w.state.properties["locked"] = false;
			},
		},
		.color{Color::HEX(0x2C2C2CF5)},
		.borderColor{Color::HEX(0x00000033)},
		.borderWidth = 1.f,
		.borderRadius = 8.f,
		.borderPosition = Box::BorderPosition::outset,
	};
	box->setChildren(Children{
		ScrollableFrame{
			.widget{
				.height = Size::Shrink,
			},
			.children{
				[&]() -> Children {
					std::vector<Child> children{};

					children.reserve(items.size());
					for (const auto &item: items) {
						if (item.content.index() == 3) {
							children.emplace_back(Box{
								.widget{
									.height = 1.f,
									.margin{1, 1, 2, 1},
								},
								.color{Color::HEX(0xFFFFFF15)},
							});
							continue;
						}
						children.emplace_back(ContextMenuButton{
							.item = item,
							.root = root,
							.menu = box,
							.menuToLock = menuToLock,
							.shouldReserveSpace = shouldReserveSpace,
							.shouldReserveEndSpace = shouldReserveEndSpace,
						});
					}

					return children;
				}(),
			},
		},
	});
	return box;
}

ContextMenu::operator Child() const {
	Child stack = GestureDetector{
		.onClick = [](Widget &w, auto &) {
			w.deleteLater();
		},
		.child{
			Stack{
				.widget{
					// .onInit = [](Widget &w) { 
					// 	w.state.properties["state"] = Storage{.stack = w.shared_from_this()};
					// 	// w.state.properties.insert({"state", Storage{}}); 
					// },
					.onUpdate = [](Widget &w) {
						auto &state = std::any_cast<Storage&>(w.state.properties.at("state"));
						for (auto &menu: state.menusToAdd) {
							w.addChild(menu);
						}
						state.menusToAdd.clear(); 
					},
				},
			},
		},
	};
	stack->state.properties["state"] = Storage{.stack = stack};
	stack->setChildren(Children{
		ContextMenuFrame{
			.position = position,
			.items = items,
			.root = stack,
		},
	});

	return stack;
}

ChildRef ContextMenu::Storage::addMenu(const Child &menu) {
	// menusToAdd.emplace_back(menu.forward());
	
	// if (Child stackWidget = stack.lock()) {
	// 	stackWidget->addChild(menu);
	// }
	menusToAdd.emplace_back(menu);
	return menu;
}

void ContextMenu::Storage::removeMenu(const ChildRef &menu) {
	if (Child menuWidget = menu.lock()) {
		menuWidget->deleteLater();
	}
}
