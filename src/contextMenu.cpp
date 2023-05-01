#include "contextMenu.hpp"
#include "align.hpp"
#include "box.hpp"
#include "child.hpp"
#include "column.hpp"
#include "gestureDetector.hpp"
#include "scrollableFrame.hpp"
#include "stack.hpp"
#include "text.hpp"
#include <memory>
#include <numeric>
#include <variant>
#include <vector>

using namespace squi;

struct ContextMenuButton {
	// Args
	const ContextMenuItem &item;
	Widget *root;
	std::shared_ptr<ContextMenu::Storage> rootState;

	struct Storage {
		// Data
		std::shared_ptr<ContextMenu::Storage> rootState;
		std::variant<std::function<void()>, std::vector<ContextMenuItem>> action;
		bool hovered = false;
		bool submenuHovered = false;
		bool submenuOpened = false;
		bool stateChanged = false;
		uint32_t submenuId = 0;

		~Storage() {
			if (submenuOpened) rootState->removeMenu(submenuId);
		}
	};

	operator Child() const;
};

struct ContextMenuFrame {
	// Args
	vec2 position;
	const std::vector<ContextMenuItem> &items;
	Widget *root;
	std::shared_ptr<ContextMenu::Storage> rootState;

	struct Storage {
		// Data
	};

	operator Child() const;
};

ContextMenuButton::operator Child() const {
	auto storage = std::make_shared<Storage>(Storage{.rootState = rootState, .action = item.action});

	return GestureDetector{
		.onEnter = [storage = storage](auto&, auto&) {
			storage->hovered = true;
			storage->stateChanged = true;
		},
		.onLeave = [storage = storage](auto&, auto&) {
			storage->hovered = false;
			storage->stateChanged = true;
		},
		.onClick = [storage = storage, root = root](auto&, auto&) { 
			if (storage->action.index() == 0)
				root->data().shouldDelete = true;
		},
		.onUpdate = [storage = storage, root = root](Widget &w, auto){
			if (!storage->stateChanged) return;
			if (!storage->hovered && !storage->submenuHovered) {
				reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0x00000000));
				if (storage->action.index() == 1 && storage->submenuOpened) {
					storage->rootState->removeMenu(storage->submenuId);
					storage->submenuOpened = false;
				}
			} else {
				reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0xFFFFFF0F));
				if (storage->action.index() == 1 && !storage->submenuOpened) {
					storage->submenuOpened = true;
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
								.items = std::get<1>(storage->action),
								.root = root,
								.rootState = storage->rootState,
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
					Align{
						.xAlign = 0.f,
						.child{
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
	auto storage = std::make_shared<Storage>();

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
					[items = items, root = root, rootState = rootState]() -> Children {
						std::vector<Child> children{};

						children.reserve(items.size());
						for (const auto &item: items) {
							children.emplace_back(ContextMenuButton{
								.item = item,
								.root = root,
								.rootState = rootState,
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

	Child stack = Stack{
		.widget{
			.onUpdate = [storage = storage](Widget &w) {
				for (auto &menu: storage->menusToAdd) {
					w.addChild(menu);
				}
				storage->menusToAdd.clear();
			},
		},
	};
	stack->setChildren(Children{
		ContextMenuFrame{
			.position = position,
			.items = items,
			.root = stack.getAddress(),
			.rootState = storage,
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
	return id++;
}

void squi::ContextMenu::Storage::removeMenu(uint32_t id) {
	if (menus.contains(id)) {
		menus.at(id)->data().shouldDelete = true;
		menus.erase(id);
	}
}
