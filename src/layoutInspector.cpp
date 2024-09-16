#include "layoutInspector.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "column.hpp"
#include "container.hpp"
#include "engine/compiledShaders/inspectorQuadfrag.hpp"
#include "engine/compiledShaders/inspectorQuadvert.hpp"
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
					if (widget.expired()) {
						w.deleteLater();
					}
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
					.darkenedBackground = false,
				},
				TextItem{
					.widget = widget,
					.title{"Should Delete"},
					.getValue = [](Widget &widget) {
						return std::format("{}", widget.isMarkedForDeletion());
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
	std::weak_ptr<LayoutInspector::Storage> state;
	float depth = 0;

	struct Storage {
		// Data
		ChildRef widget;
		float depth;
		std::weak_ptr<LayoutInspector::Storage> state;
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
		Observable<bool> setVisible{};
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
						.onUpdate = [storage, setVisible](Widget &w) {
							if (storage->stateChanged) {
								w.setChildren(Children{
									Align{
										.child{
											FontIcon{
												.icon = storage->expanded ? char32_t{0xE972} : char32_t{0xE974},
												.size = 12.f,
											},
										},
									},
								});
								storage->stateChanged = false;
							}

							auto widget = storage->widget.lock();
							if (widget && !widget->getChildren().empty()) {
								// w.getChildren().front()->flags.visible = true;
								setVisible.notify(true);
							} else {
								// w.getChildren().front()->flags.visible = false;
								setVisible.notify(false);
							}
						},
					},
					.child{
						Align{
							.child{
								FontIcon{
									.widget{
										.onInit = [setVisible](Widget &w) {
											w.customState.add(setVisible.observe([&w](bool visible) {
												w.flags.visible = visible;
											}));
										},
									},
									.icon = storage->expanded ? char32_t{0xE972} : char32_t{0xE974},
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
						state.lock()->hoveredWidget = storage->widget;
						storage->hovered = true;
						event.widget.reDraw();
					},
					.onLeave = [storage, state = state](GestureDetector::Event event) {
						if (auto shared = state.lock()) {
							storage->hovered = false;
							if (shared->hoveredWidget.lock() == storage->widget.lock()) {
								shared->hoveredWidget.reset();
							}
							event.widget.reDraw();
						}
					},
					.onClick = [storage](auto event) {
						if (auto shared = storage->state.lock()) {
							shared->selectedWidget = storage->widget;
							shared->selectedWidgetChanged = true;
							shared->activeButton = event.widget.weak_from_this();
						}
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
										if (storage->hovered || state.lock()->activeButton.lock() == w.shared_from_this())
											return Color(0xFFFFFF0D);

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
	std::weak_ptr<LayoutInspector::Storage> storage;
	// Args
	operator Child() const {
		return GestureDetector{
			.onClick = [storage = storage](auto) {
				if (auto shared = storage.lock()) {
					shared->pauseUpdates = !shared->pauseUpdates;
					shared->pauseUpdatesChanged = true;
				}
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
							if (auto shared = storage.lock()) {
								if (shared->pauseUpdatesChanged) {
									w.setChildren(Children{
										Align{
											.child{
												FontIcon{
													.icon = shared->pauseUpdates ? char32_t{0xE768} : char32_t{0xE769},
													.size = 16.f,
												},
											},
										},
									});
									shared->pauseUpdatesChanged = false;
								}
							}
						},
					},
					.color{Color(0x00000000)},
					.borderRadius{4.f},
					.child = Align{
						.child{
							FontIcon{
								.icon = 0xE769,
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
	std::weak_ptr<LayoutInspector::Storage> storage;

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
								if (auto shared = storage.lock()) {
									shared->pauseLayout = !shared->pauseLayout;
									shared->pauseLayoutChanged = true;
								}
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
									if (auto shared = storage.lock()) {
										if (auto content = shared->contentStack.lock()) {
											for (auto &child: content->getChildren()) {
												if (test(child)) {
													w.addChild(LayoutItem{child->shared_from_this(), shared});
												}
											}
										}
										if (auto overlays = shared->overlayStack.lock()) {
											for (auto &child: overlays->getChildren()) {
												if (test(child)) {
													w.addChild(LayoutItem{child->shared_from_this(), shared});
												}
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
							if (auto shared = storage.lock()) {
								if (!shared->selectedWidget.expired() && shared->selectedWidgetChanged) {
									w.setChildren(Children{TextItems{shared->selectedWidget}});
									shared->selectedWidgetChanged = false;
								}
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
			.onInit = [storage](Widget &w) {
				auto &window = Window::of(&w);
				storage->pipeline = window.pipelineStore.getPipeline(Store::PipelineProvider<InspectorPipeline>{
					.key = "squiInspectorPipeline",
					.provider = [&]() {
						return InspectorPipeline::Args{
							.vertexShader = Engine::Shaders::inspectorQuadvert,
							.fragmentShader = Engine::Shaders::inspectorQuadfrag,
							.instance = window.engine.instance,
						};
					},
				});
			},
		},
		.children{
			Stack{
				.widget{
					.width = Size::Expand,
					.height = Size::Expand,
					.onUpdate = [storage](Widget &w) {
						auto &inputState = InputState::of(&w);
						w.flags.shouldUpdateChildren = !storage->pauseUpdates || inputState.isKey(GLFW_KEY_F10, GLFW_PRESS);
						w.flags.shouldLayoutChildren = !storage->pauseLayout || inputState.isKey(GLFW_KEY_F11, GLFW_PRESS);
						w.flags.shouldArrangeChildren = !storage->pauseLayout || inputState.isKey(GLFW_KEY_F11, GLFW_PRESS);
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
								storage->addedChildrenObserver = addedChildren.observe([weakStorage = std::weak_ptr<Storage>(storage)](const Child &child) {
									if (auto storage = weakStorage.lock()) {
										if (auto content = storage->contentStack.lock()) {
											content->addChild(child);
										}
									}
								});
							},
							.onUpdate = [storage](Widget &w) {
								if (InputState::of(&w).isKey(GLFW_KEY_F9, GLFW_PRESS)) w.layout(w.getLayoutSize(), {}, {}, true);
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
								storage->addedOverlaysObserver = addedOverlays.observe([weakStorage = std::weak_ptr<Storage>(storage)](const Child &child) {
									if (auto storage = weakStorage.lock()) {
										if (auto overlays = storage->overlayStack.lock()) {
											overlays->addChild(child);
										}
									}
								});
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
					if (event.state.isKeyPressedOrRepeat(GLFW_KEY_F5)) {
						storage->pauseUpdates = !storage->pauseUpdates;
						storage->pauseUpdatesChanged = true;
					}
					if (event.state.isKeyPressedOrRepeat(GLFW_KEY_F6)) {
						storage->pauseLayout = !storage->pauseLayout;
					}
					if (event.state.isKeyPressedOrRepeat(GLFW_KEY_F12)) {
						event.widget.flags.visible = !*event.widget.flags.visible;
					}
					if (event.state.isKeyPressedOrRepeat(GLFW_KEY_I, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT)) {
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
							.afterDraw = [storage](Widget & /*w*/) {
								Child widget = storage->hoveredWidget.lock();
								if (!widget) return;
								if (*widget->flags.visible) {
									if (!storage->pipeline) return;
									Engine::InspectorQuad quad{Engine::InspectorQuad::Args{
										.position = widget->getPos(),
										.size = widget->getLayoutSize(),
										.margins = *widget->state.margin,
										.paddings = *widget->state.padding,
									}};
									storage->pipeline->bind();
									auto [vi, ii] = storage->pipeline->getIndexes();
									storage->pipeline->addData(quad.getData(vi, ii));
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
