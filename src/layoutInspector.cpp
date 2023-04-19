#include "layoutInspector.hpp"
#include "align.hpp"
#include "box.hpp"
#include "column.hpp"
#include "scrollableFrame.hpp"
#include "text.hpp"
#include "window.hpp"

using namespace squi;

struct LayoutItem {
	// Args
	Widget &widget;

	struct Storage {
		// Data
	};

	operator Child() const {
		auto storage = std::make_shared<Storage>();

		return Column{
			.widget{
				.height = Size::Shrink,
			},
			.children{
				Box{
					.widget{
						.width = Size::Expand,
						.height = 32.f,
						.margin{2},
						.padding{0, 0, 0, 4},
					},
					.color{Color::HEX(0xFFFFFF0D)},
					.borderColor{Color::HEX(0x0000001A)},
					.borderWidth = 1.0f,
					.borderRadius = 4.0f,
					.child{
						Align{
							.xAlign = 0.0f,
							.child{
								Text{
									.text{typeid(widget).name()},
								},
							},
						},
					},
				},
				[&]() {
					auto &children = widget.getChildren();

					if (children.empty()) {
						return Child{};
					}

					return Child(Column{
						.widget{
							.height = Size::Shrink,
							.margin{0, 0, 0, 4},
						},
						.children{
							[&]() {
								auto &children = widget.getChildren();

								std::vector<Child> result{};
								result.reserve(children.size());

								for (auto &child: children) {
									result.push_back(LayoutItem{*child});
								}

								return Children(result);
							}(),
						},
					});
				}(),
			},
		};
	}
};

LayoutInspector::operator Child() const {
	auto storage = std::make_shared<Storage>();

	return Align{
		.child = Box{
			.widget{
				.width = 400.f,
				.height = 400.f,
			},
			.color{Color::HEX(0x202020CC)},
			.borderColor{Color::HEX(0x75757566)},
			.borderWidth = 1.0f,
			.borderRadius = 8.0f,
			.child{
				ScrollableFrame{
					.children{
						[window = window]() {
							// auto &w = (Window&)*window;
							auto &children = window->getChildren();

							std::vector<Child> result;
							result.reserve(children.size());

							for (auto &child: children) {
								result.push_back(LayoutItem{*child});
							}

							return Children(result);
						}(),
					},
				},
			},
		},
	};
}
