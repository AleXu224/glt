#include "contextMenu.hpp"
#include "align.hpp"
#include "box.hpp"
#include "column.hpp"
#include "gestureDetector.hpp"
#include "stack.hpp"
#include "text.hpp"

using namespace squi;

struct ContextMenuButton {
	// Args
	std::string_view text;

	struct Storage {
		// Data
	};

	operator Child() const {
		auto storage = std::make_shared<Storage>();

		return GestureDetector{
			.onEnter = [](auto &w, auto) { reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0xFFFFFF0F)); },
			.onLeave = [](auto &w, auto) { reinterpret_cast<Box::Impl &>(w).setColor(Color::HEX(0x00000000)); },
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
									.text{text},
								},
							},
						},
					},
				},
			},
		};
	}
};

ContextMenu::operator Child() const {
	auto storage = std::make_shared<Storage>();

	return Stack{
		.children{
			Box{
				.widget{
					.width = 50.f,
					.height = 100.f,
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
						},
						.children{
							ContextMenuButton{
								.text{"Copy"},
							},
							ContextMenuButton{
								.text{"Paste"},
							},
							ContextMenuButton{
								.text{"Save As..."},
							},
						},
					},
				},
			},
		},
	};
}
