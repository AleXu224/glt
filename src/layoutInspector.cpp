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
#include <GLFW/glfw3.h>
#include <algorithm>
#include <any>
#include <chrono>
#include <format>
#include <functional>
#include <memory>
#include <string_view>
#include <utility>



using namespace squi;

struct TextItem {
	// Args
	ChildRef widget;
	std::string_view title;
	std::function<std::string(Widget &)> getValue{};
	bool darkenedBackground = false;

	operator Child() const {
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
									if (Child widget = target.lock()) {
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
	ChildRef widget;

	operator Child() const {
		return Column{
			.widget{
				.width = Size::Expand,
				.height = Size::Shrink,
				.onUpdate = [widget = widget](Widget &w) {
					if (widget.expired()) w.deleteLater();
				},
			},
			.children{
				// TextItem{
				// 	.widget = widget,
				// 	.title{"ID"},
				// 	.getValue = [](Widget &widget) {
				// 		return std::format("{}", widget.id);
				// 	},
				// },
				TextItem{
					.widget = widget,
					.title{"Position"},
					.getValue = [](Widget &widget) {
						return std::format("x: {}, y: {}", widget.getPos().x, widget.getPos().y);
					},
				},
				TextItem{
					.widget = widget,
					.title{"Size"},
					.getValue = [](Widget &widget) {
						return std::format("w: {}, h: {}", widget.getSize().x, widget.getSize().y);
					},
					.darkenedBackground = true,
				},
				TextItem{
					.widget = widget,
					.title{"Margin"},
					.getValue = [](Widget &widget) {
						return std::format("l: {}, t: {}, r: {}, b: {}", widget.state.margin.left, widget.state.margin.top, widget.state.margin.right, widget.state.margin.bottom);
					},
				},
				TextItem{
					.widget = widget,
					.title{"Padding"},
					.getValue = [](Widget &widget) {
						return std::format("l: {}, t: {}, r: {}, b: {}", widget.state.padding.left, widget.state.padding.top, widget.state.padding.right, widget.state.padding.bottom);
					},
					.darkenedBackground = true,
				},
				TextItem{
					.widget = widget,
					.title{"Visible"},
					.getValue = [](Widget &widget) {
						return std::format("{}", widget.flags.visible);
					},
					.darkenedBackground = true,
				},
			},
		};
	}
};

struct LayoutItem {
	// Args
	ChildRef widget;
	std::shared_ptr<LayoutInspector::Storage> state;
	float depth = 0;

	struct Storage {
		// Data
		ChildRef widget;
		float depth;
		std::shared_ptr<LayoutInspector::Storage> state;
		std::chrono::time_point<std::chrono::steady_clock> timeCreated = std::chrono::steady_clock::now();
		bool expanded = false;
		bool hovered = false;
		bool stateChanged = false;
	};

	static Child buttonFactory(std::shared_ptr<Storage> &storage) {
		if (Child widget = storage->widget.lock(); !widget || widget->getChildren().empty()) {
			return Container{
				.widget{
					.width = 32.f,
					.height = Size::Expand,
				},
			};
		}
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
			.widget = widget,
			.depth = depth,
			.state = state,
		});

		return Column{
			.widget{
				.height = Size::Shrink,
				.onInit = [storage](Widget &w) { w.state.properties.insert({"layoutItem", storage}); },
				.onUpdate = [storage](Widget &w) {
					if (storage->widget.expired()) w.deleteLater(); },
			},
			.children{
				GestureDetector{
					.onEnter = [storage, state = state](Widget &w, auto &) { 
						state->hoveredWidget = storage->widget;
						storage->hovered = true; },
					.onLeave = [storage, state = state](Widget &w, auto &) {
						storage->hovered = false;
						if (state->hoveredWidget.lock() == storage->widget.lock()) {
							state->hoveredWidget.reset();
						} },
					.onClick = [storage](Widget &w, auto &) {
						storage->state->selectedWidget = storage->widget;
						storage->state->selectedWidgetChanged = true;
						storage->state->activeButton = w.shared_from_this(); },
					.child{
						Box{
							.widget{
								.width = Size::Expand,
								.height = 28.f,
								.margin{4, 2},
								.padding{0.f, 0.f, 0.f, depth * 16.f},
								.onUpdate = [storage = storage, state = state](Widget &w) {
									Color outputColor = [&]() {
										if (storage->hovered || state->activeButton.lock() == w.shared_from_this())
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
												Child widget = storage->widget.lock();
												if (!widget) return "null";
												auto &w = *widget;
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
					Child widget = storage->widget.lock();
					if (!widget) return Child{};
					auto &children = widget->getChildren();

					if (children.empty()) {
						return Child{};
					}

					return Column{
						.widget{
							.height = Size::Shrink,
							.onUpdate = [storage](Widget &w) {
								w.flags.visible = storage->expanded;
								if (!storage->expanded) return;
								Child widget = storage->widget.lock();
								if (!widget) return;
								const auto &children = w.getChildren();
								const auto test = [&children](Child &child) -> bool {
									auto result = std::find_if(children.begin(), children.end(), [child = child](const auto &w) {
										return std::any_cast<std::shared_ptr<LayoutItem::Storage>>(w->state.properties.at("layoutItem"))->widget.lock() == child->shared_from_this();
									});
									return result == children.end();
								};
								for (auto &child: widget->getChildren()) {
									if (test(child)) {
										w.addChild(LayoutItem{child->shared_from_this(), storage->state, storage->depth + 1});
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
									result.push_back(LayoutItem{child->shared_from_this(), state, depth + 1});
								}

								return Children(std::move(result));
							}(),
						},
					};
				}(),
			},
		};
	}
};

struct LayoutInspectorActionButton {
	std::shared_ptr<LayoutInspector::Storage> storage;
	// Args
	operator Child() const {
		return GestureDetector{
			.onClick = [storage = storage](Widget &w, auto) {
				storage->pauseUpdates = !storage->pauseUpdates;
				storage->pauseUpdatesChanged = true;
			},
			.child{
				Box{
					.widget{
						.width = 36.f,
						.height = 36.f,
						.margin = 2.f,
						.onUpdate = [storage = storage](Widget &w) {
							auto &box = dynamic_cast<Box::Impl &>(w);
							auto &gd = std::any_cast<GestureDetector::Storage &>(w.state.properties.at("gestureDetector"));
							if (gd.hovered || gd.focused)
								box.setColor(Color::HEX(0xFFFFFF0D));
							else
								box.setColor(Color::HEX(0x00000000));

							if (storage->pauseUpdatesChanged) {
								w.setChildren(Children{
									Align{
										.child{
											FontIcon{
												.icon{storage->pauseUpdates ? "\uE768" : "\uE769"},
												.font{R"(C:\Windows\Fonts\segmdl2.ttf)"},
												.size = 16.f,
											},
										},
									},
								});
								storage->pauseUpdatesChanged = false;
							}
						},
					},
					.color{Color::HEX(0x00000000)},
					.borderRadius = 4.f,
					.child = Align{
						.child{
							FontIcon{
								.icon{"\uE769"},
								.font{R"(C:\Windows\Fonts\segmdl2.ttf)"},
								.size = 16.f,
							},
						},
					},
				},
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
				Row{
					.widget{
						.height = 48.f,
						.padding = 4.f,
					},
					.children{
						LayoutInspectorActionButton{storage},
					},
				},
				Box{
					.widget{
						.height = 1.f,
						.margin = {1, 0, 2, 0},
					},
					.color{Color::HEX(0xFFFFFF15)},
				},
				ScrollableFrame{
					.children{
						Column{
							.widget{
								.width = Size::Expand,
								.height = Size::Shrink,
								.onUpdate = [storage = storage](Widget &w) {
									const auto &children = w.getChildren();
									const auto test = [&children](Child &child) -> bool {
										auto result = std::find_if(children.begin(), children.end(), [child = child](const auto &w) {
											return std::any_cast<std::shared_ptr<LayoutItem::Storage>>(w->state.properties.at("layoutItem"))->widget.lock() == child;
										});
										return result == children.end();
									};
									for (auto &child: storage->content) {
										if (test(child)) {
											w.addChild(LayoutItem{child->shared_from_this(), storage});
										}
									}
									for (auto &child: storage->overlays) {
										if (test(child)) {
											w.addChild(LayoutItem{child->shared_from_this(), storage});
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
							if (!storage->selectedWidget.expired() && storage->selectedWidgetChanged) {
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
					.onUpdate = [storage](Widget &w) {
						w.flags.shouldUpdateChildren = !storage->pauseUpdates;
					},
				},
				.children{
					// Content
					Stack{
						.widget{
							.width = Size::Expand,
							.height = Size::Expand,
							.onUpdate = [storage](Widget &w) {
								if (storage->content.empty()) return;
								for (auto &child: storage->content) {
									w.addChild(child);
								}
								storage->content.clear();
							},
						},
					},
					// Overlays
					Stack{
						.widget{
							.width = Size::Expand,
							.height = Size::Expand,
							.onUpdate = [storage](Widget &w) {
								if (storage->overlays.empty()) return;
								for (auto &child: storage->overlays) {
									w.addChild(child);
								}
								storage->overlays.clear();
							},
						},
					},
				},
			},
			GestureDetector{
				.onLeave = [storage](Widget &w, auto &) { storage->hoveredWidget.reset(); },
				.onUpdate = [storage](Widget &w, auto &gd) { 
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_F5)) {
						storage->pauseUpdates = !storage->pauseUpdates;
						storage->pauseUpdatesChanged = true;
					}
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_F12)) {
						w.flags.visible = !w.flags.visible;
					} },
				.child{
					Box{
						.widget{
							.width = 400.f,
							.height = Size::Expand,
							.margin{4.f},
							.padding{1.f},
							.afterDraw = [storage](Widget &w) {
								Child widget = storage->hoveredWidget.lock();
								if (!widget) return;
								if (widget->flags.visible) {
									Quad previewQuad{Quad::Args{
										.pos = widget->getPos() + widget->state.margin.getPositionOffset(),
										.size = widget->getSize(),
										.color = Color::HEX(0x00008040),
									}};

									Quad paddingQuad{Quad::Args{
										.pos = widget->getPos() + widget->state.padding.getPositionOffset() + widget->state.margin.getPositionOffset(),
										.size = widget->getSize() - widget->state.padding.getSizeOffset(),
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
