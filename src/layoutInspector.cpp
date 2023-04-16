#include "layoutInspector.hpp"
#include "align.hpp"
#include "box.hpp"
#include "column.hpp"
#include "scrollableFrame.hpp"
#include "sizeBehavior.hpp"
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
				.sizeBehavior{
					.horizontal = SizeBehaviorType::FillParent,
					.vertical = SizeBehaviorType::MatchChild,
				},
			},
			.children{
				Box{
					.widget{
						.size{32},
						.margin{2},
						.padding{0, 0, 0, 4},
						.sizeBehavior{
							.horizontal = SizeBehaviorType::FillParent,
						},
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
				Column{
					.widget{
						.margin{0, 0, 0, 4},
						.sizeBehavior{
							.horizontal = SizeBehaviorType::FillParent,
							.vertical = SizeBehaviorType::MatchChild,
						},
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
						}()},
				},
			},
		};
	}
};

LayoutInspector::operator Child() const {
	auto storage = std::make_shared<Storage>();

	return Align{
		.child = Box{
			.widget{
				.size{400, 400},
			},
			.color{Color::HEX(0x202020CC)},
			.borderColor{Color::HEX(0x75757566)},
			.borderWidth = 1.0f,
			.borderRadius = 8.0f,
			.child{
				ScrollableFrame{
					.widget{
						.sizeBehavior{
							.horizontal = SizeBehaviorType::FillParent,
							.vertical = SizeBehaviorType::FillParent,
						},
					},
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
