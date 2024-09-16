#include "contextMenu.hpp"
#include "box.hpp"
#include "container.hpp"
#include "fontIcon.hpp"
#include "gestureDetector.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "stack.hpp"
#include "text.hpp"
#include "vec2.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <print>
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
		.onEnter = [storage = storage](auto) {
			storage->hovered = true;
			storage->stateChanged = true;
		},
		.onLeave = [storage = storage](auto) {
			storage->hovered = false;
			storage->stateChanged = true;
		},
		.onClick = [storage = storage, root = root](GestureDetector::Event /*event*/) {
			if (Child rootWidget = root.lock()) {
				switch (storage->content.index()) {
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
		.onUpdate = [storage = storage, root = root](GestureDetector::Event event) {
			Child rootWidget = root.lock();
			if (!rootWidget) return;
			auto &rootState = rootWidget->customState.get<ContextMenu::Storage>("state");
			if (storage->content.index() == 1 && storage->submenuOpened) {
				if (Child menuWidget = storage->menu.lock()) {
					// const auto &menuLocked = std::any_cast<bool &>(menuWidget->state.properties.at("locked"));
					const auto &menuLocked = menuWidget->customState.get<bool>("locked");
					if (menuLocked) return;
				}
			}
			if (!storage->stateChanged) return;
			if (!storage->hovered && !storage->submenuHovered) {
				reinterpret_cast<Box::Impl &>(event.widget).setColor(0x00000000);
				if (storage->content.index() == 1 && storage->submenuOpened) {
					ContextMenu::Storage::removeMenu(storage->submenu);
					storage->submenuOpened = false;
					if (storage->menuToLock.has_value()) {
						if (Child menuToLockWidget = storage->menuToLock.value().lock()) {
							auto &locked = menuToLockWidget->customState.get<bool>("locked");
							locked = false;
						}
					}
				}
			} else {
				reinterpret_cast<Box::Impl &>(event.widget).setColor(0xFFFFFF0F);
				if (storage->content.index() == 1 && !storage->submenuOpened) {
					storage->submenuOpened = true;
					if (storage->menuToLock.has_value()) {
						if (Child menuToLockWidget = storage->menuToLock.value().lock()) {
							auto &locked = menuToLockWidget->customState.get<bool>("locked");
							locked = true;
						}
					}
					storage->submenuHovered = false;
					const auto &rect = event.widget.getRect();
					storage->submenu = rootState.addMenu(GestureDetector{
						.onEnter = [storage = storage](auto) {
							storage->submenuHovered = true;
							storage->stateChanged = true;
						},
						.onLeave = [storage = storage](auto) {
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
			}
		},
		.child{
			Box{
				.widget{
					.height = 28.f,
					.margin = Margin{5.f, 2.f},
					.padding = Padding{4.f, 0.f},
				},
				.color{Color(0)},
				.borderRadius{4.f},
				.child{
					Row{
						.alignment = Row::Alignment::center,
						.children{
							[&]() -> Child {
								switch (item.content.index()) {
									case 2: {// Toggle
										return FontIcon{
											.margin{0.f, 14.f, 0.f, 9.f},
											.icon = 0xF78C,
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
										}
										return {};
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
											.icon = 0xE974,
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
										}
										return {};
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
	const bool shouldReserveSpace = std::ranges::any_of(items, [](const auto &item) {
		return item.content.index() == 2;
	});
	const bool shouldReserveEndSpace = std::ranges::any_of(items, [](const auto &item) {
		return item.content.index() == 1;
	});
	Child box = Box{
		.widget{
			.width = Size::Shrink,
			.height = Size::Shrink,
			.sizeConstraints{
				.minWidth = 100.f,
			},
			.padding = Padding{1.f, 4.f},
			.onInit = [](Widget &w) {
				w.customState.add("locked", false);
			},
			.onArrange = [position = position](Widget &w, vec2 &pos) {
				auto rect = Rect::fromPosSize(position, w.getLayoutRect().size());
				const auto parentRect = w.state.parent->getContentRect();
				vec2 newPos = position;

				if (rect.right > parentRect.right) {
					newPos.x -= rect.right - parentRect.right;
				}
				if (rect.bottom > parentRect.bottom) {
					newPos.y -= rect.bottom - parentRect.bottom;
				}

				pos = newPos;
			},
		},
		.color{Color(0x2C2C2CF5)},
		.borderColor{Color(0x00000033)},
		.borderWidth{1.f},
		.borderRadius{8.f},
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
									.margin = Margin{1, 1, 2, 1},
								},
								.color{Color(0xFFFFFF15)},
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
	Child stack = Stack{
		.widget{
			.onUpdate = [](Widget &w) {
				auto &state = w.customState.get<Storage>("state");
				for (auto &menu: state.menusToAdd) {
					w.addChild(menu);
				}
				state.menusToAdd.clear();
			},
		},
	};
	stack->customState.add("state", Storage{.stack = stack});
	stack->setChildren(Children{
		GestureDetector{
			.onClick = [](GestureDetector::Event event) {
				event.widget.state.parent->deleteLater();
			},
			.child = Container{},
		},
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
