#include "layoutInspector.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "column.hpp"
#include "container.hpp"
#include "fontIcon.hpp"
#include "gestureDetector.hpp"
#include "inspectorQuad.hpp"
#include "row.hpp"
#include "scrollableFrame.hpp"
#include "stack.hpp"
#include "text.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <chrono>
#include <format>
#include <functional>
#include <memory>
#include <print>
#include <string_view>
#include <utility>
#include "engine/compiledShaders/inspectorQuadfrag.hpp"
#include "engine/compiledShaders/inspectorQuadvert.hpp"

using namespace squi;
LayoutInspector::InspectorPipeline * LayoutInspector::pipeline = nullptr;

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
			.color{darkenedBackground ? Color(0x00000040) : Color(0x00000000)},
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
						return std::format("l: {}, t: {}, r: {}, b: {}", widget.state.margin->left, widget.state.margin->top, widget.state.margin->right, widget.state.margin->bottom);
					},
				},
				TextItem{
					.widget = widget,
					.title{"Padding"},
					.getValue = [](Widget &widget) {
						return std::format("l: {}, t: {}, r: {}, b: {}", widget.state.padding->left, widget.state.padding->top, widget.state.padding->right, widget.state.padding->bottom);
					},
					.darkenedBackground = true,
				},
				TextItem{
					.widget = widget,
					.title{"Visible"},
					.getValue = [](Widget &widget) {
						return std::format("{}", *widget.flags.visible);
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
		// if (Child widget = storage->widget.lock(); !widget || widget->getChildren().empty()) {
		// 	return Container{
		// 		.widget{
		// 			.width = 32.f,
		// 			.height = Size::Expand,
		// 		},
		// 	};
		// }
		return GestureDetector{
			.onClick = [storage](auto) {
				storage->expanded = !storage->expanded;
				storage->stateChanged = true;
			},
			.child{
				Container{
					.widget{
						.width = 32.f,
						.height = Size::Expand,
						.padding = Padding{0.f, 14.f, 0.f, 6.f},
						.onUpdate = [storage](Widget &w) {
							if (storage->stateChanged) {
								w.setChildren(Children{
									Align{
										.child{
											FontIcon{
												.icon = storage->expanded ? char32_t{0xE972} : char32_t{0xE974},
												.font{R"(C:\Windows\Fonts\segmdl2.ttf)"},
												.size = 12.f,
											},
										},
									},
								});
								storage->stateChanged = false;
							}

							auto widget = storage->widget.lock();
							if (widget && !widget->getChildren().empty()) {
								w.getChildren().front()->flags.visible = true;
							} else {
								w.getChildren().front()->flags.visible = false;
							}
						},
					},
					.child{
						Align{
							.child{
								FontIcon{
									.icon = storage->expanded ? char32_t{0xE972} : char32_t{0xE974},
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
				.onInit = [storage](Widget &w) {
					w.customState.add("layoutItem", storage);
				},
				.onUpdate = [storage](Widget &w) {
					if (storage->widget.expired()) w.deleteLater();
				},
			},
			.children{
				GestureDetector{
					.onEnter = [storage, state = state](GestureDetector::Event event) {
						state->hoveredWidget = storage->widget;
						storage->hovered = true;
						event.widget.reDraw();
					},
					.onLeave = [storage, state = state](GestureDetector::Event event) {
						storage->hovered = false;
						if (state->hoveredWidget.lock() == storage->widget.lock()) {
							state->hoveredWidget.reset();
						}
						event.widget.reDraw();
					},
					.onClick = [storage](auto event) {
						storage->state->selectedWidget = storage->widget;
						storage->state->selectedWidgetChanged = true;
						storage->state->activeButton = event.widget.shared_from_this();
					},
					.child{
						Box{
							.widget{
								.width = Size::Expand,
								.height = 28.f,
								.margin = Margin{4, 2},
								.padding = Padding{0.f, 0.f, 0.f, depth * 16.f},
								.onUpdate = [storage = storage, state = state](Widget &w) {
									Color outputColor = [&]() {
										if (storage->hovered || state->activeButton.lock() == w.shared_from_this())
											return Color(0xFFFFFF0D);
										else
											return Color(0x00000000);
									}();
									const auto timeAlive = std::chrono::steady_clock::now() - storage->timeCreated;
									if (timeAlive < std::chrono::milliseconds(200)) {
										outputColor = outputColor.transistion(Color(0x17C55DAA), 1.f - (static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(timeAlive).count()) / 200.f));
									}
									auto &box = dynamic_cast<Box::Impl &>(w);
									box.setColor(outputColor);
								},
							},
							.color{Color(0x00000000)},
							.borderRadius{4.0f},
							.child{
								Row{
									.widget{
										.padding = Padding{8.f, 0},
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
				Column{
					.widget{
						.height = Size::Shrink,
						.onUpdate = [storage](Widget &w) {
							w.flags.visible = storage->expanded;
							if (!storage->expanded) return;
							Child widget = storage->widget.lock();
							if (!widget) return;
							const auto &children = w.getChildren();
							const auto test = [&children](Child &child) -> bool {
								auto result = std::find_if(children.begin(), children.end(), [child = child](const Child &w) {
									return w->customState.get<std::shared_ptr<LayoutItem::Storage>>("layoutItem")->widget.lock() == child->shared_from_this();
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
							auto w = widget.lock();
							if (!w) return Children{};
							auto &children = w->getChildren();

							std::vector<Child> result{};
							result.reserve(children.size());

							for (auto &child: children) {
								result.push_back(LayoutItem{child->shared_from_this(), state, depth + 1});
							}

							return Children(std::move(result));
						}(),
					},
				},
			},
		};
	}
};

struct LayoutInspectorActionButton {
	std::shared_ptr<LayoutInspector::Storage> storage;
	// Args
	operator Child() const {
		return GestureDetector{
			.onClick = [storage = storage](auto) {
				storage->pauseUpdates = !storage->pauseUpdates;
				storage->pauseUpdatesChanged = true;
			},
			.onUpdate = [](auto event) {
				auto &box = dynamic_cast<Box::Impl &>(event.widget);

				if (event.state.hovered || event.state.focused)
					box.setColor(Color(0xFFFFFF0D));
				else
					box.setColor(Color(0x00000000));
			},
			.child{
				Box{
					.widget{
						.width = 36.f,
						.height = 36.f,
						.margin = 2.f,
						.onUpdate = [storage = storage](Widget &w) {
							if (storage->pauseUpdatesChanged) {
								w.setChildren(Children{
									Align{
										.child{
											FontIcon{
												.icon = storage->pauseUpdates ? char32_t{0xE768} : char32_t{0xE769},
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
					.color{Color(0x00000000)},
					.borderRadius{4.f},
					.child = Align{
						.child{
							FontIcon{
								.icon = 0xE769,
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
					.alignment = Row::Alignment::center,
					.children{
						LayoutInspectorActionButton{storage},
						Button{
							.text{"Pause Layout"},
							.style = ButtonStyle::Standard(),
							.onClick = [storage = storage](auto) {
								storage->pauseLayout = !storage->pauseLayout;
								storage->pauseLayoutChanged = true;
							},
						},
					},
				},
				Box{
					.widget{
						.height = 1.f,
						.margin = Margin{1, 0, 2, 0},
					},
					.color{Color(0xFFFFFF15)},
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
										auto result = std::find_if(children.begin(), children.end(), [child = child](const Child &w) {
											return w->customState.get<std::shared_ptr<LayoutItem::Storage>>("layoutItem")->widget.lock() == child;
										});
										return result == children.end();
									};
									if (auto content = storage->contentStack.lock()) {
										for (auto &child: content->getChildren()) {
											if (test(child)) {
												w.addChild(LayoutItem{child->shared_from_this(), storage});
											}
										}
									}
									if (auto overlays = storage->overlayStack.lock()) {
										for (auto &child: overlays->getChildren()) {
											if (test(child)) {
												w.addChild(LayoutItem{child->shared_from_this(), storage});
											}
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
					.color{Color(0x00000033)},
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
					.color{Color(0x00000015)},
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
	auto storage = std::make_shared<Storage>();

	return Row{
		.widget{
			.onDraw = [](Widget &w) {
				if (!LayoutInspector::pipeline) {
					pipeline = &Window::of(&w).engine.instance.createPipeline<InspectorPipeline>(InspectorPipeline::Args{
						.vertexShader = Engine::Shaders::inspectorQuadvert,
						.fragmentShader = Engine::Shaders::inspectorQuadfrag,
						.instance = Window::of(&w).engine.instance,
					});
				}
			}
		},
		.children{
			Stack{
				.widget{
					.width = Size::Expand,
					.height = Size::Expand,
					.onUpdate = [storage](Widget &w) {
						w.flags.shouldUpdateChildren = !storage->pauseUpdates || GestureDetector::isKey(GLFW_KEY_F10, GLFW_PRESS);
						w.flags.shouldLayoutChildren = !storage->pauseLayout || GestureDetector::isKey(GLFW_KEY_F11, GLFW_PRESS);
						w.flags.shouldArrangeChildren = !storage->pauseLayout || GestureDetector::isKey(GLFW_KEY_F11, GLFW_PRESS);
					},
				},
				.children{
					// Content
					Stack{
						.widget{
							.width = Size::Expand,
							.height = Size::Expand,
							.onInit = [addedChildren = addedChildren, storage](Widget &w) {
								auto shared = w.weak_from_this();
								storage->contentStack = shared;
								if (auto addedChildrenEvent = addedChildren.lock()) {
									storage->addedChildrenObserver = addedChildrenEvent->observe([w = shared](const Child &child) {
										if (auto widget = w.lock()) {
											widget->addChild(child);
										}
									});
								}
							},
							.onUpdate = [storage](Widget &w) {
								if (GestureDetector::isKey(GLFW_KEY_F9, GLFW_PRESS)) w.layout(w.getLayoutSize(), {});
							},
						},
					},
					// Overlays
					Stack{
						.widget{
							.width = Size::Expand,
							.height = Size::Expand,
							.onInit = [addedOverlays = addedOverlays, storage](Widget &w) {
								auto shared = w.weak_from_this();
								storage->overlayStack = shared;
								if (auto addedOverlaysEvent = addedOverlays.lock()) {
									storage->addedOverlaysObserver = addedOverlaysEvent->observe([w = shared](const Child &child) {
										if (auto widget = w.lock()) {
											widget->addChild(child);
										}
									});
								}
							},
						},
					},
				},
			},
			GestureDetector{
				.onLeave = [storage](GestureDetector::Event event) {
					storage->hoveredWidget.reset();
					event.widget.reDraw();
				},
				.onUpdate = [storage](auto event) {
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_F5)) {
						storage->pauseUpdates = !storage->pauseUpdates;
						storage->pauseUpdatesChanged = true;
					}
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_F6)) {
						storage->pauseLayout = !storage->pauseLayout;
					}
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_F12)) {
						event.widget.flags.visible = !*event.widget.flags.visible;
					}
					if (GestureDetector::isKey(GLFW_KEY_I, GLFW_PRESS, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT)) {
						event.widget.flags.visible = !*event.widget.flags.visible;
					}
				},
				.child{
					Box{
						.widget{
							.width = 400.f,
							.height = Size::Expand,
							.margin{4.f},
							.padding{1.f},
							.onInit = [](Widget &w) {
								w.flags.visible = false;
							},
							.afterDraw = [storage](Widget &w) {
								Child widget = storage->hoveredWidget.lock();
								if (!widget) return;
								if (*widget->flags.visible) {
									Engine::InspectorQuad quad{Engine::InspectorQuad::Args{
										.position = widget->getPos(),
										.size = widget->getLayoutSize(),
										.margins = *widget->state.margin,
										.paddings = *widget->state.padding,
									}};
									LayoutInspector::pipeline->bind();
									auto [vi, ii] = LayoutInspector::pipeline->getIndexes();
									LayoutInspector::pipeline->addData(quad.getData(vi, ii));
								}
							},
						},
						.color{Color(0x1C1C1CFF)},
						.borderColor{Color(0x00000033)},
						.borderWidth{1.f},
						.borderRadius{8.f},
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
