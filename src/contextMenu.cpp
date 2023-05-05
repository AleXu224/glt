#include "contextMenu.hpp"
#include "align.hpp"
#include "box.hpp"
#include "child.hpp"
#include "column.hpp"
#include "gestureDetector.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "stack.hpp"
#include "text.hpp"
#include <memory>
#include <numeric>
#include <optional>
#include <variant>
#include <vector>

using namespace squi;

struct ContextMenuButton {
	// Args
	const ContextMenu::Item &item;
	Widget *root;
	uint32_t menuId;
	std::shared_ptr<ContextMenu::Storage> rootState;
	std::optional<uint32_t> menuToLock{};

	struct Storage {
		// Data
		std::optional<uint32_t> menuToLock{};
		uint32_t menuId;
		std::shared_ptr<ContextMenu::Storage> rootState;
		std::variant<std::function<void()>, ContextMenu::Item::Submenu, ContextMenu::Item::Toggle> content;
		bool hovered = false;
		bool submenuHovered = false;
		bool submenuOpened = false;
		bool stateChanged = false;
		uint32_t submenuId = 0;
	};

	operator Child() const;
};

struct ContextMenuFrame {
	// Args
	vec2 position;
	const std::vector<ContextMenu::Item> &items;
	Widget *root;
	std::shared_ptr<ContextMenu::Storage> rootState;
	uint32_t id;
	std::optional<uint32_t> menuToLock{};

	struct Storage {
		// Data
		uint32_t id;
	};

	operator Child() const;
};

ContextMenuButton::operator Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.menuToLock = menuToLock,
		.menuId = menuId,
		.rootState = rootState,
		.content = item.content,
	});

	return GestureDetector{
		.onEnter = [storage = storage](auto&, auto&) {
			storage->hovered = true;
			storage->stateChanged = true;
		},
		.onLeave = [storage = storage](auto&, auto&) {
			storage->hovered = false;
			storage->stateChanged = true;
		},
		.onClick = [storage = storage, root = root](Widget &, GestureDetector::Storage &) { 
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
			}
		},
		.onUpdate = [storage = storage, root = root](Widget &w, auto){
			if (storage->content.index() == 1 && storage->submenuOpened) {
				if (storage->rootState->locked.contains(storage->menuId) && storage->rootState->locked.at(storage->menuId)) {
					return;
				}
			}
			if (!storage->stateChanged) return;
			if (!storage->hovered && !storage->submenuHovered) {
				reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0x00000000));
				if (storage->content.index() == 1 && storage->submenuOpened) {
					storage->rootState->removeMenu(storage->submenuId);
					storage->submenuOpened = false;
					if (storage->menuToLock.has_value()) {
						storage->rootState->locked[storage->menuToLock.value()] = false;
					}
				}
			} else {
				reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0xFFFFFF0F));
				if (storage->content.index() == 1 && !storage->submenuOpened) {
					storage->submenuOpened = true;
					if (storage->menuToLock.has_value()) {
						storage->rootState->locked[storage->menuToLock.value()] = true;
					}
					storage->submenuHovered = false;
					const auto &rect = w.getRect();
					storage->submenuId = storage->rootState->addMenu(GestureDetector{
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
								.rootState = storage->rootState,
								.id = storage->rootState->id,
								.menuToLock = storage->menuId,
							},
						},
					});
				}
			}
		},
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
							Box{
								.widget{
									.width = 12.f,
									.height = 12.f,
								},
							},
							Text{
								.text{item.text},
							},
						},
					},
				},
			},
		},
	};
}

ContextMenuFrame::operator Child() const {
	auto storage = std::make_shared<Storage>(Storage{.id = id});

	return Box{
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
		.borderPosition = squi::Box::BorderPosition::outset,
		.child{
			ScrollableFrame{
				.widget{
					.height = Size::Shrink,
				},
				.children{
					[items = items, root = root, rootState = rootState, storage = storage, menuToLock = menuToLock]() -> Children {
						std::vector<Child> children{};

						children.reserve(items.size());
						for (const auto &item: items) {
							children.emplace_back(ContextMenuButton{
								.item = item,
								.root = root,
								.menuId = storage->id,
								.rootState = rootState,
								.menuToLock = menuToLock,
							});
						}

						return children;
					}(),
				},
			},
		},
	};
}

ContextMenu::operator Child() const {
	auto storage = std::make_shared<Storage>();

	Child stack = GestureDetector{
		.onClick = [](Widget &w, auto&) {
			w.data().shouldDelete = true;
		},
		.child{
			Stack{
				.widget{
					.onUpdate = [storage = storage](Widget &w) {
						for (auto &menu: storage->menusToAdd) {
							w.addChild(menu);
						}
						storage->menusToAdd.clear();
					},
				},
			},
		},
	};
	stack->setChildren(Children{
		ContextMenuFrame{
			.position = position,
			.items = items,
			.root = stack.getAddress(),
			.rootState = storage,
			.id = storage->id,
		},
	});

	return stack;
}

std::optional<std::shared_ptr<Widget>> squi::ContextMenu::Storage::getMenu(uint32_t id) {
	if (auto it = menus.find(id); it != menus.end())
		return it->second;
	else
		return std::nullopt;
}

uint32_t squi::ContextMenu::Storage::addMenu(const Child &menu) {
	menusToAdd.emplace_back(menu);
	menus[id] = menu;
	locked[id] = false;
	return id++;
}

void squi::ContextMenu::Storage::removeMenu(uint32_t id) {
	if (menus.contains(id)) {
		menus.at(id)->data().shouldDelete = true;
		menus.erase(id);
	}
	if (locked.contains(id)) {
		locked.erase(id);
	}
}
