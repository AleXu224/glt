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
	uint64_t rootID;
	uint64_t menuId;
	std::optional<uint64_t> menuToLock{};
	// In the case of a context menu that has one or more items that have an icon
	// then all text will need to have the same horizontal position.
	bool shouldReserveSpace = false;
	bool shouldReserveEndSpace = false;

	struct Storage {
		// Data
		std::optional<uint64_t> menuToLock{};
		uint64_t menuId;
		std::variant<std::function<void()>, ContextMenu::Item::Submenu, ContextMenu::Item::Toggle, ContextMenu::Item::Divider> content;
		bool hovered = false;
		bool submenuHovered = false;
		bool submenuOpened = false;
		bool stateChanged = false;
		uint64_t submenuId = 0;
	};

	operator Child() const;
};

struct ContextMenuFrame {
	// Args
	vec2 position;
	const std::vector<ContextMenu::Item> &items;
	uint64_t rootID;
	std::optional<uint64_t> menuToLock{};

	operator Child() const;
};

ContextMenuButton::operator Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.menuToLock = menuToLock,
		.menuId = menuId,
		.content = item.content,
	});

	return GestureDetector{
		.onEnter = [storage = storage](auto &, auto &) {
			storage->hovered = true;
			storage->stateChanged = true; },
		.onLeave = [storage = storage](auto &, auto &) {
			storage->hovered = false;
			storage->stateChanged = true; },
		.onClick = [storage = storage, rootID = rootID](Widget &, GestureDetector::Storage &) { 
			auto root = Widget::Store::getWidget(rootID);
			if (!root) return;
			switch(storage->content.index()) {
				case 0: {
					root->data().shouldDelete = true;
					auto &f = std::get<0>(storage->content);
					if (f) f();
					break;
				}
				case 2: {
					root->data().shouldDelete = true;
					auto &t = std::get<2>(storage->content);
					t.value = !t.value;
					if (t.callback) t.callback(t.value);
				}
			} },
		.onUpdate = [storage = storage, rootID = rootID](Widget &w, auto) {
			auto root = Widget::Store::getWidget(rootID);
			if (!root) return;
			auto &rootState = std::any_cast<ContextMenu::Storage&>(root->data().properties.at("state"));
			if (storage->content.index() == 1 && storage->submenuOpened) {
				if (rootState.locked.contains(storage->menuId) && rootState.locked.at(storage->menuId)) {
					return;
				}
			}
			if (!storage->stateChanged) return;
			if (!storage->hovered && !storage->submenuHovered) {
				reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0x00000000));
				if (storage->content.index() == 1 && storage->submenuOpened) {
					rootState.removeMenu(storage->submenuId);
					storage->submenuOpened = false;
					if (storage->menuToLock.has_value()) {
						rootState.locked[storage->menuToLock.value()] = false;
					}
				}
			} else {
				reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0xFFFFFF0F));
				if (storage->content.index() == 1 && !storage->submenuOpened) {
					storage->submenuOpened = true;
					if (storage->menuToLock.has_value()) {
						rootState.locked[storage->menuToLock.value()] = true;
					}
					storage->submenuHovered = false;
					const auto &rect = w.getRect();
					storage->submenuId = rootState.addMenu(GestureDetector{
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
								.rootID = rootID,
								.menuToLock = storage->menuId,
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
							.rootID = rootID,
							.menuId = box->id,
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
			w.data().shouldDelete = true;
		},
		.child{
			Stack{
				.widget{
					.onInit = [](Widget &w) { w.data().properties.insert({"state", Storage{}}); },
					.onUpdate = [](Widget &w) {
						auto &state = std::any_cast<Storage&>(w.data().properties.at("state"));
						for (auto &menu: state.menusToAdd) {
							w.addChild(menu);
						}
						state.menusToAdd.clear(); },
				},
			},
		},
	};
	stack->setChildren(Children{
		ContextMenuFrame{
			.position = position,
			.items = items,
			.rootID = stack->id,
		},
	});

	return stack;
}

uint64_t squi::ContextMenu::Storage::addMenu(const Child &menu) {
	menusToAdd.emplace_back(menu);
	locked.insert({menu->id, false});
	return menu->id;
}

void squi::ContextMenu::Storage::removeMenu(uint64_t id) {
	auto widget = Widget::Store::getWidget(id);
	if (widget) {
		widget->data().shouldDelete = true;
	}
	if (locked.contains(id)) {
		locked.erase(id);
	}
}
