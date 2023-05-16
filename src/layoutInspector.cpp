#include "layoutInspector.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "column.hpp"
#include "container.hpp"
#include "fontIcon.hpp"
#include "gestureDetector.hpp"
#include "quad.hpp"
#include "ranges"
#include "renderer.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "stack.hpp"
#include "text.hpp"
#include <chrono>
#include <algorithm>
#include <chrono>
#include <format>
#include <functional>
#include <memory>
#include <string_view>
#include <utility>


using namespace squi;

struct TextItem {
	// Args
	uint64_t widget;
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
									if (auto widget = Widget::Store::getWidget(target); widget) {
										auto newText = getValue ? getValue(*widget) : "";
										if (oldText != newText) {
											w.setText(newText);
										}
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
	uint64_t widgetID;

	operator Child() const {
		return Column{
			.widget{
				.width = Size::Expand,
				.height = Size::Shrink,
				.onUpdate = [widgetID = widgetID](Widget &w) {
					auto widget = Widget::Store::getWidget(widgetID);
					if (!widget) w.data().shouldDelete = true;
				},
			},
			.children{
				TextItem{
					.widget = widgetID,
					.title{"ID"},
					.getValue = [](Widget &widget) {
						return std::format("{}", widget.id);
					},
				},
				TextItem{
					.widget = widgetID,
					.title{"Position"},
					.getValue = [](Widget &widget) {
						return std::format("x: {}, y: {}", widget.getPos().x, widget.getPos().y);
					},
				},
				TextItem{
					.widget = widgetID,
					.title{"Size"},
					.getValue = [](Widget &widget) {
						return std::format("w: {}, h: {}", widget.getSize().x, widget.getSize().y);
					},
					.darkenedBackground = true,
				},
				TextItem{
					.widget = widgetID,
					.title{"Margin"},
					.getValue = [](Widget &widget) {
						return std::format("l: {}, t: {}, r: {}, b: {}", widget.data().margin.left, widget.data().margin.top, widget.data().margin.right, widget.data().margin.bottom);
					},
				},
				TextItem{
					.widget = widgetID,
					.title{"Padding"},
					.getValue = [](Widget &widget) {
						return std::format("l: {}, t: {}, r: {}, b: {}", widget.data().padding.left, widget.data().padding.top, widget.data().padding.right, widget.data().padding.bottom);
					},
					.darkenedBackground = true,
				},
				TextItem{
					.widget = widgetID,
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
	uint64_t widgetID;
	std::shared_ptr<LayoutInspector::Storage> state;
	float depth = 0;

	struct Storage {
		// Data
		uint64_t widgetID;
		float depth;
		std::shared_ptr<LayoutInspector::Storage> state;
		std::chrono::time_point<std::chrono::steady_clock> timeCreated = std::chrono::steady_clock::now();
		bool expanded = false;
		bool hovered = false;
		bool stateChanged = false;
	};

	static Child buttonFactory(std::shared_ptr<Storage> &storage) {
		auto widget = Widget::Store::getWidget(storage->widgetID);
		if (!widget || widget->getChildren().empty()) return {};
		return GestureDetector{
			.onClick = [storage](Widget &, auto) {
				storage->expanded = !storage->expanded;
				storage->stateChanged = true;
			},
			.child{
				Container{
					.widget{
						.width = Size::Shrink,
						.height = Size::Expand,
						.padding{0.f, 14.f, 0.f, 6.f},
						.onUpdate = [storage](Widget &w) {
							if (storage->stateChanged) {
								w.setChildren(Children{
									Align{
										.child{
											FontIcon{
												.icon{storage->expanded ? "\uE972" : "\uE974"},
												.font{R"(C:\Windows\Fonts\segmdl2.ttf)"},
												.size = 12.f,
											},
										},
									},
								});
								storage->stateChanged = false;
							}
						},
					},
					.child{
						Align{
							.child{
								FontIcon{
									.icon{storage->expanded ? "\uE972" : "\uE974"},
									.font{R"(C:\Windows\Fonts\segmdl2.ttf)"},
									.size = 12.f,
								},
							},
						},
					},
				},
			},
		};
	}

	operator Child() const {
		auto storage = std::make_shared<Storage>(Storage{
			.widgetID = widgetID,
			.depth = depth,
			.state = state,
		});

		return Column{
			.widget{
				.height = Size::Shrink,
				.onInit = [storage](Widget &w) {
					w.data().properties.insert({"layoutItem", storage});
				},
				.onUpdate = [storage](Widget &w) {
					auto widget = Widget::Store::getWidget(storage->widgetID);
					if (!widget) w.data().shouldDelete = true;
				},
			},
			.children{
				GestureDetector{
					.onEnter = [storage, state = state](Widget &w, auto &) { 
						state->hoveredWidgetID = storage->widgetID;
						storage->hovered = true; },
					.onLeave = [storage, state = state](Widget &w, auto &) {
						storage->hovered = false;
						if (state->hoveredWidgetID == storage->widgetID) {
							state->hoveredWidgetID = 0;
						} },
					.onClick = [storage](Widget &w, auto &) {
						storage->state->selectedWidgetID = storage->widgetID;
						storage->state->selectedWidgetChanged = true;
						storage->state->activeButtonID = w.id; },
					.child{
						Box{
							.widget{
								.width = Size::Expand,
								.height = 28.f,
								.margin{4, 2},
								.padding{0.f, 0.f, 0.f, depth * 16.f},
								.onUpdate = [storage = storage, state = state](Widget &w) {
									Color outputColor = [&](){
										if (storage->hovered || state->activeButtonID == w.id)
											return Color::HEX(0xFFFFFF0D);
										else
											return Color::HEX(0x00000000);
									}();
									const auto timeAlive = std::chrono::steady_clock::now() - storage->timeCreated;
									if (timeAlive < std::chrono::milliseconds(200)) {
										outputColor = outputColor.transistion(Color::HEX(0x17C55DAA), 1.f - (static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(timeAlive).count()) / 200.f));
									}
									auto &box = dynamic_cast<Box::Impl &>(w);
									box.setColor(outputColor);
								},
							},
							.color{Color::HEX(0x00000000)},
							// .borderColor{Color::HEX(0x0000001A)},
							// .borderWidth = 1.0f,
							.borderRadius = 4.0f,
							.child{
								Row{
									.widget{
										.padding{8.f, 0},
									},
									.alignment = Row::Alignment::center,
									.children{
										buttonFactory(storage),
										Text{
											.text{[&]() -> std::string {
												auto wPtr = Widget::Store::getWidget(storage->widgetID);
												if (!wPtr) return "null";
												auto &w = *wPtr;
												return typeid(w).name();
											}()},
											.fontSize = 14.f,
										},
									},
								},
							},
						},
					},
				},
				[&]() -> Child {
					auto widget = Widget::Store::getWidget(storage->widgetID);
					if (!widget) return Child{};
					auto &children = widget->getChildren();

					if (children.empty()) {
						return Child{};
					}

					return Column{
						.widget{
							.height = Size::Shrink,
							.onUpdate = [storage](Widget &w) {
								w.data().visible = storage->expanded;
								auto widget = Widget::Store::getWidget(storage->widgetID);
								if (!widget) return;
								const auto &children = w.getChildren();
								const auto test = [children](std::shared_ptr<Widget> &child) -> bool {
									auto result = std::find_if(children.begin(), children.end(), [id = child->id](const auto &w) {
										return std::any_cast<std::shared_ptr<LayoutItem::Storage>>(w->data().properties.at("layoutItem"))->widgetID == id;
									});
									return result == children.end();
								};
								for (auto &child: widget->getChildren()) {
									if (test(child)) {
										w.addChild(LayoutItem{child->id, storage->state, storage->depth + 1});
									}
								}
							},
						},
						.children{
							[&]() {
								auto &children = widget->getChildren();

								std::vector<Child> result{};
								result.reserve(children.size());

								for (auto &child: children) {
									result.push_back(LayoutItem{child->id, state, depth + 1});
								}

								return Children(result);
							}(),
						},
					};
				}(),
			},
		};
	}
};

struct LayoutInspectorContent {
	// Args
	std::shared_ptr<LayoutInspector::Storage> storage;

	operator Child() const {
		return Column{
			.children{
				ScrollableFrame{
					.children{
						Column{
							.widget{
								.width = Size::Expand,
								.height = Size::Shrink,
								.onUpdate = [storage = storage](Widget &w) {
									const auto &children = w.getChildren();
									const auto test = [children](std::shared_ptr<Widget> &child) -> bool {
										auto result = std::find_if(children.begin(), children.end(), [id = child->id](const auto &w) {
											return std::any_cast<std::shared_ptr<LayoutItem::Storage>>(w->data().properties.at("layoutItem"))->widgetID == id;
										});
										return result == children.end();
									};
									for (auto &child: storage->content) {
										if (test(child)) {
											w.addChild(LayoutItem{child->id, storage});
										}
									}
									for (auto &child: storage->overlays) {
										if (test(child)) {
											w.addChild(LayoutItem{child->id, storage});
										}
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
						.onUpdate = [storage = storage](Widget &w) {
							if (storage->selectedWidgetID && storage->selectedWidgetChanged) {
								w.setChildren(Children{TextItems{storage->selectedWidgetID}});
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
				.onLeave = [storage](Widget &w, auto &) {
					storage->hoveredWidgetID = 0;
				},
				.child{
					Box{
						.widget{
							.width = 400.f,
							.height = Size::Expand,
							.margin{4.f},
							.padding{1.f},
							.afterDraw = [storage](Widget &w) {
								if (!storage->hoveredWidgetID) return;
								auto widget = Widget::Store::getWidget(storage->hoveredWidgetID);
								if (!widget) return;
								if (widget && widget->data().visible) {
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
						.color{Color::HEX(0x1C1C1CFF)},
						.borderColor{Color::HEX(0x00000033)},
						.borderWidth = 1.f,
						.borderRadius = 8.f,
						.borderPosition = squi::Box::BorderPosition::outset,
						.child{
							LayoutInspectorContent{storage},
						},
					},
				},
			},
		},
	};
}
