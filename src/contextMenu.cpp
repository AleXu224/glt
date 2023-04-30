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
	std::shared_ptr<ContextMenu::Storage> state;

	struct Storage {
		// Data
		std::variant<std::function<void()>, std::vector<ContextMenuItem>> action;
		bool submenuHovered = false;
		uint32_t submenuId = 0;
	};

	operator Child() const;
};

struct ContextMenuFrame {
	// Args
	vec2 position;
	const std::vector<ContextMenuItem> &items;
	Widget *root;
	std::shared_ptr<ContextMenu::Storage> state;

	struct Storage {
		// Data
	};

	operator Child() const;
};

ContextMenuButton::operator Child() const {
	auto storage = std::make_shared<Storage>(Storage{.action = item.action});

	return GestureDetector{
		.onEnter = [storage = storage, root = root, state = state](Widget &w, auto) { 
				reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0xFFFFFF0F));
				if (storage->action.index() == 1) {
					storage->submenuHovered = false;
					const auto &rect = w.getRect();
					storage->submenuId = state->addMenu(GestureDetector{
						.onEnter = [storage = storage](Widget &w, auto &s){
							storage->submenuHovered = true;
						},
						.child{
							ContextMenuFrame{
								.position = vec2{rect.right, rect.top},
								.items = std::get<1>(storage->action),
								.root = root,
								.state = state,
							},
						},
					});
				} },
		.onLeave = [storage = storage, state = state](auto &w, auto) { 
			reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0x00000000)); 
			if (storage->action.index() == 1) {
				state->removeMenu(storage->submenuId);
			} },
		.onClick = [storage = storage, root = root](auto &w, auto &s) { 
				if (storage->action.index() == 0)
					root->data().shouldDelete = true; },
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
					[items = items, root = root, state = state]() -> Children {
						std::vector<Child> children{};

						children.reserve(items.size());
						for (const auto &item: items) {
							children.emplace_back(ContextMenuButton{
								.item = item,
								.root = root,
								.state = state,
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
			.state = storage,
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
