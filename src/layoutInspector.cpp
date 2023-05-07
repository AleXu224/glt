#include "layoutInspector.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "column.hpp"
#include "container.hpp"
#include "gestureDetector.hpp"
#include "quad.hpp"
#include "renderer.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "stack.hpp"
#include "text.hpp"
#include <format>
#include <functional>
#include <memory>
#include <string_view>

using namespace squi;

static std::weak_ptr<Widget> activeWidget{};

struct TextItem {
	// Args
	std::shared_ptr<Widget> widget{};
	std::string_view title;
	std::function<std::string(Widget &)> getValue{};
	bool darkenedBackground = false;
	struct Storage {
		// Data
	};

	operator Child() const {
		auto storage = std::make_shared<Storage>();

		return Box{
			.widget{
				.height = Size::Shrink,
				.padding = 8,
			},
			.color{darkenedBackground ? Color::HEX(0x00000040) : Color::HEX(0x00000000)},
			.child{
				Row{
					.widget{
						.height = Size::Shrink,
					},
					.children{
						Text{.text{title}},
						Container{.widget{.height = Size::Shrink}},
						Text{
							.widget{
								.onUpdate = [getValue = getValue, target = widget](Widget &widget) {
									auto &w = dynamic_cast<Text::Impl &>(widget);
									auto oldText = w.getText();
									auto newText = getValue ? getValue(*target) : "";
									if (oldText != newText) {
										w.setText(newText);
									}
								},
							},
						},
					},
				},
			},
		};
	}
};

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
				TextItem{
					.widget{widget},
					.title{"Position"},
					.getValue = [](Widget &widget) {
						return std::format("x: {}, y: {}", widget.getPos().x, widget.getPos().y);
					},
				},
				TextItem{
					.widget{widget},
					.title{"Size"},
					.getValue = [](Widget &widget) {
						return std::format("w: {}, h: {}", widget.getSize().x, widget.getSize().y);
					},
					.darkenedBackground = true,
				},
				TextItem{
					.widget{widget},
					.title{"Margin"},
					.getValue = [](Widget &widget) {
						return std::format("l: {}, t: {}, r: {}, b: {}", widget.data().margin.left, widget.data().margin.top, widget.data().margin.right, widget.data().margin.bottom);
					},
				},
				TextItem{
					.widget{widget},
					.title{"Padding"},
					.getValue = [](Widget &widget) {
						return std::format("l: {}, t: {}, r: {}, b: {}", widget.data().padding.left, widget.data().padding.top, widget.data().padding.right, widget.data().padding.bottom);
					},
					.darkenedBackground = true,
				},
				TextItem{
					.widget{widget},
					.title{"Should delete"},
					.getValue = [](Widget &widget) {
						return std::format("{}", widget.data().shouldDelete);
					},
				},
			},
		};
	}
};

struct LayoutItem {
	// Args
	std::shared_ptr<Widget> widget;
	std::shared_ptr<LayoutInspector::Storage> state;

	struct Storage {
		// Data
		std::shared_ptr<Widget> widget;
		std::shared_ptr<LayoutInspector::Storage> state;
	};

	operator Child() const {
		auto storage = std::make_shared<Storage>(Storage{
			.widget = widget,
			.state = state,
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
					.onLeave = [storage](auto &, auto&) {
						if (activeWidget.lock() == storage->widget) {
							activeWidget.reset();
						}
					},
					.onClick = [storage](auto &, auto &) {
						storage->state->selectedWidget = storage->widget;
						storage->state->selectedWidgetChanged = true;
					},
					.child{
						Box{
							.widget{
								.width = Size::Expand,
								.height = Size::Shrink,
								.margin{2},
							},
							.color{Color::HEX(0xFFFFFF0D)},
							// .borderColor{Color::HEX(0x0000001A)},
							// .borderWidth = 1.0f,
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
											result.push_back(LayoutItem{child, state});
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
	auto storage = std::make_shared<Storage>(Storage{
		.content = content,
		.overlays = overlays,
	});

	return Row{
		.children{
			Stack{
				.widget{
					.width = Size::Expand,
					.height = Size::Expand,
				},
				.children{
					// Content
					Stack{
						.widget{
							.width = Size::Expand,
							.height = Size::Expand,
							.onUpdate = [storage](Widget &w) {
								w.setChildren(storage->content);
							},
						},
					},
					// Overlays
					Stack{
						.widget{
							.width = Size::Expand,
							.height = Size::Expand,
							.onUpdate = [storage](Widget &w) {
								w.setChildren(storage->overlays);
							},
						},
					},
				},
			},
			GestureDetector{
				.onLeave = [](Widget &w, auto &) {
					activeWidget.reset();
				},
				.child{
					Box{
						.widget{
							.width = 400.f,
							.height = Size::Expand,
							.margin{4.f},
							.padding{1.f},
							.afterDraw = [](Widget &w) {
								auto widget = activeWidget.lock();
								if (widget) {
									Quad previewQuad{Quad::Args{
										.pos = widget->getPos() + widget->data().margin.getPositionOffset(),
										.size = widget->getSize(),
										.color = Color::HEX(0x00008040),
									}};

									Quad paddingQuad{Quad::Args{
										.pos = widget->getPos() + widget->data().padding.getPositionOffset() + widget->data().margin.getPositionOffset(),
										.size = widget->getSize() - widget->data().padding.getSizeOffset(),
										.color = Color::HEX(0x0008040),
									}};

									const auto layoutRect = widget->getLayoutRect();
									Quad layoutQuad{Quad::Args{
										.pos = widget->getPos(),
										.size = layoutRect.size(),
										.color = Color::HEX(0x008A0040),
									}};

									auto &renderer = Renderer::getInstance();
									renderer.addQuad(layoutQuad);
									renderer.addQuad(previewQuad);
									renderer.addQuad(paddingQuad);
								}
							},
						},
						.color{Color::HEX(0x2C2C2CF5)},
						.borderColor{Color::HEX(0x00000033)},
						.borderWidth = 1.f,
						.borderRadius = 8.f,
						.borderPosition = squi::Box::BorderPosition::outset,
						.child{
							Column{
								.children{
									ScrollableFrame{
										.children{
											Button{
												.width = Size::Expand,
												.margin{4.f},
												.text{"Update tree"},
												.onClick = [storage]() {
													storage->shouldUpdate = true;
												},
											},
											Column{
												.widget{
													.width = Size::Expand,
													.height = Size::Shrink,
													.onUpdate = [storage](Widget &w) {
														if (storage->shouldUpdate) {
															std::vector<Child> result;
															result.reserve(storage->content.size() + storage->overlays.size());

															for (auto &child: storage->content) {
																result.push_back(LayoutItem{child, storage});
															}

															for (auto &overlay: storage->overlays) {
																result.push_back(LayoutItem{overlay, storage});
															}

															w.setChildren(Children(result));
															storage->shouldUpdate = false;
														}
													},
												},
											},
										},
									},
									Box{
										.widget{
											.width = Size::Expand,
											.height = 1.f,
										},
										.color{Color::HEX(0x00000033)},
									},
									Box{
										.widget{
											.height = 256.f,
											.onUpdate = [storage](Widget &w) {
												if (storage->selectedWidget && storage->selectedWidgetChanged) {
													w.setChildren(Children{TextItems{storage->selectedWidget}});
													storage->selectedWidgetChanged = false;
												}
											},
										},
										.color{Color::HEX(0x00000015)},
										.child{
											Align{
												.child{Text{.text{"Select a widget to show info"}}},
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
	};
}
