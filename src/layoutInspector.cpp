#include "layoutInspector.hpp"
#include "align.hpp"
#include "box.hpp"
#include "column.hpp"
#include "gestureDetector.hpp"
#include "quad.hpp"
#include "renderer.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "text.hpp"
#include "window.hpp"
#include <format>
#include <memory>

using namespace squi;

static std::weak_ptr<Widget> activeWidget{};

struct TextItems {
	// Args
	std::shared_ptr<Widget> widget;

	operator Child() const {
		return Column{
			.widget{
				.width = Size::Expand,
				.height = Size::Shrink,
			},
			.children{
				Text{
					.widget{
						.onUpdate = [target = widget](Widget &widget) {
							auto &w = dynamic_cast<Text::Impl &>(widget);

							w.setText(std::format("x: {}, y: {}", target->getPos().x, target->getPos().y));
						},
					},
					.text{std::format("x: {}, y: {}", widget->getPos().x, widget->getPos().y)},
				},
				Text{
					.widget{
						.onUpdate = [target = widget](Widget &widget) {
							auto &w = dynamic_cast<Text::Impl &>(widget);

							w.setText(std::format("w: {}, h: {}", target->getSize().x, target->getSize().y));
						},
					},
					.text{std::format("w: {}, h: {}", widget->getSize().x, widget->getSize().y)},
				},
				Text{
					.widget{
						.onUpdate = [target = widget](Widget &widget) {
							auto &w = dynamic_cast<Text::Impl &>(widget);

							w.setText(std::format("margin: l:{} t:{} r:{} b:{}", target->data().margin.left, target->data().margin.top, target->data().margin.right, target->data().margin.bottom));
						},
					},
					.text{std::format("margin: l:{} t:{} r:{} b:{}", widget->data().margin.left, widget->data().margin.top, widget->data().margin.right, widget->data().margin.bottom)},
				},
				Text{
					.widget{
						.onUpdate = [target = widget](Widget &widget) {
							auto &w = dynamic_cast<Text::Impl &>(widget);

							w.setText(std::format("padding: l:{} t:{} r:{} b:{}", target->data().padding.left, target->data().padding.top, target->data().padding.right, target->data().padding.bottom));
						},
					},
					.text{std::format("padding: l:{} t:{} r:{} b:{}", widget->data().padding.left, widget->data().padding.top, widget->data().padding.right, widget->data().padding.bottom)},
				},
			},
		};
	}
};

struct LayoutItem {
	// Args
	std::shared_ptr<Widget> widget;

	struct Storage {
		// Data
		std::shared_ptr<Widget> widget;
	};

	operator Child() const {
		auto storage = std::make_shared<Storage>(Storage{
			.widget = widget,
		});

		return Column{
			.widget{
				.height = Size::Shrink,
			},
			.children{
				GestureDetector{
					.onEnter = [storage](Widget &w, auto &) {
						activeWidget = storage->widget;
					},
					.child{
						Box{
							.widget{
								.width = Size::Expand,
								.height = Size::Shrink,
								.margin{2},
							},
							.color{Color::HEX(0xFFFFFF0D)},
							.borderColor{Color::HEX(0x0000001A)},
							.borderWidth = 1.0f,
							.borderRadius = 4.0f,
							.child{
								Column{
									.children{
										Text{
											.widget{
												.margin{8.f},
											},
											.text{[&]() {
												auto &w = *widget;
												return typeid(w).name();
											}()},
										},
										Box{
											.widget{
												.width = Size::Expand,
												.height = Size::Shrink,
												.padding{8.f, 4.f},
											},
											.color{Color::HEX(0x0000001A)},
											.child{
												TextItems{widget},
											},
										},
									},
								},
							},
						},
					},
				},
				[&]() -> Child {
					auto &children = widget->getChildren();

					if (children.empty()) {
						return Child{};
					}

					return Row{
						.widget{
							.width = Size::Expand,
							.height = Size::Shrink,
						},
						.children{
							Box{
								.widget{
									.width = 2.f,
									.height = Size::Expand,
									.margin{0, 0, 0, 4},
								},
							},
							Column{
								.widget{
									.height = Size::Shrink,
									.margin{0, 0, 0, 4},
								},
								.children{
									[&]() {
										auto &children = widget->getChildren();

										std::vector<Child> result{};
										result.reserve(children.size());

										for (auto &child: children) {
											result.push_back(LayoutItem{child});
										}

										return Children(result);
									}(),
								},
							},
						},
					};
				}(),
			},
		};
	}
};

LayoutInspector::operator Child() const {
	auto storage = std::make_shared<Storage>();

	return Align{
		.child = GestureDetector{
			.onLeave = [](Widget &w, auto &) {
				activeWidget.reset();
			},
			.child{
				Box{
					.widget{
						.width = 400.f,
						.height = 400.f,
						.padding{1.f},
						.afterDraw = [](Widget &w) {
							auto widget = activeWidget.lock();
							if (widget) {
								Quad previewQuad{Quad::Args{
									.pos = widget->getPos() + widget->data().margin.getPositionOffset(),
									.size = widget->getSize(),
									.color = Color::HEX(0x008AFF40),
								}};

								auto &renderer = Renderer::getInstance();
								renderer.addQuad(previewQuad);
							}
						},
					},
					.color{Color::HEX(0x202020CC)},
					.borderColor{Color::HEX(0x75757566)},
					.borderWidth = 1.0f,
					.borderRadius = 8.0f,
					.child{
						ScrollableFrame{
							.children{
								[window = window]() {
									auto &w = (Window &) *window;
									auto &children = window->getChildren();

									std::vector<Child> result;
									result.reserve(children.size());

									for (auto &child: children) {
										result.push_back(LayoutItem{child});
									}

									auto &overlays = w.getOverlays();

									for (auto &overlay: overlays) {
										result.push_back(LayoutItem{overlay->getChildren().at(0)});
									}

									return Children(result);
								}(),
							},
						},
					},
				},
			},
		},
	};
}
