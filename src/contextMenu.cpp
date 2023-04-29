#include "contextMenu.hpp"
#include "align.hpp"
#include "box.hpp"
#include "child.hpp"
#include "column.hpp"
#include "gestureDetector.hpp"
#include "stack.hpp"
#include "text.hpp"
#include <numeric>
#include <vector>

using namespace squi;

struct ContextMenuButton {
	// Args
	const ContextMenuItem &item;
	Widget *root;

	struct Storage {
		// Data
	};

	operator Child() const {
		auto storage = std::make_shared<Storage>();

		return GestureDetector{
			.onEnter = [](auto &w, auto) { reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0xFFFFFF0F)); },
			.onLeave = [](auto &w, auto) { reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0x00000000)); },
			.onClick = [root = root](auto &w, auto &s){
				root->data().shouldDelete = true;
				printf("Clicked\n");
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
};

struct ContextMenuFrame {
	// Args
	Widget::Args widget;
	const std::vector<ContextMenuItem> &items;
	Widget *root;

	struct Storage {
		// Data
	};

	operator Child() const {
		auto storage = std::make_shared<Storage>();

		return Box{
			.widget{
				.width = Size::Shrink,
				.height = Size::Shrink,
				.padding{1.f, 4.f},
			},
			.color{Color::HEX(0x2C2C2C26)},
			.borderColor{Color::HEX(0x00000033)},
			.borderWidth = 1.f,
			.borderRadius = 8.f,
			.child{
				Column{
					.widget{
						.height = Size::Shrink,
						.onLayout = [](Widget &w, auto &maxSize, auto &minSize) {
							maxSize.x = w.getMinWidth();
						},
					},
					.children{
						[items = items, root = root]() -> Children {
							std::vector<Child> children{};

							children.reserve(items.size());
							for (const auto &item: items) {
								// if (std::holds_alternative<std::function<void()>>(item.action)) {
								// 	children.emplace_back(ContextMenuButton{
								// 		.item = item,
								// 	});
								// } else {
								children.emplace_back(ContextMenuButton{
									.item = item,
									.root = root,
								});
								// }
							}

							return children;
						}(),
					},
				},
			},
		};
	}
};

ContextMenu::operator Child() const {
	auto storage = std::make_shared<Storage>();

	Child stack = Stack{};
	stack->setChildren(Children{
		ContextMenuFrame{
			.items = items,
			.root = stack.getAddress(),
		},
	});

	return stack;
}
