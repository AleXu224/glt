#include "layoutInspector.hpp"
#include "compiledShaders/inspectorQuadfrag.hpp"
#include "compiledShaders/inspectorQuadvert.hpp"
#include "inspectorQuad.hpp"
#include "navigator.hpp"
#include "pipeline.hpp"
#include "widgets/button.hpp"
#include "widgets/column.hpp"
#include "widgets/fontIcon.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/row.hpp"
#include "widgets/scrollview.hpp"
#include "widgets/stack.hpp"
#include "widgets/transform.hpp"
#include "widgets/wrapper.hpp"

#include "utils.hpp"

#include "core/app.hpp"

namespace squi {
	//
	[[nodiscard]] std::string LayoutInspectorItem::State::getElementName(const RenderObjectPtr &renderObject) {
		std::string ret{renderObject->element->widget->getName()};
		if (const auto found = ret.find("struct"); found != std::string::npos) {
			ret.erase(found, 6);
		}
		if (const auto found = ret.find("squi::"); found != std::string::npos) {
			ret.erase(found, 6);
		}
		if (const auto found = ret.find("core::"); found != std::string::npos) {
			ret.erase(found, 6);
		}
		return ret;
	}

	core::Child LayoutInspectorItem::State::build(const Element &) {
		auto renderObject = widget->renderObjectPtr.lock();
		if (!renderObject) return nullptr;

		auto childrenCount = renderObject->getChildren().size();

		return Column{
			.children{
				Gesture{
					.onEnter = [this](auto &&) {
						widget->onHoverRenderObject({true, this->widget->renderObjectPtr});
					},
					.onLeave = [this](auto &&) {
						widget->onHoverRenderObject({false, this->widget->renderObjectPtr});
					},
					.child = Button{
						.widget{
							.width = Size::Expand,
						},
						.theme = Button::Theme::Subtle(),
						.onClick = [this]() {
							setState([&]() {
								open = !open;
								iconRotate = open ? 90.f : 0.f;
								iconOffset = open ? .5f : 0.f;
							});
							if (widget->onSelectRenderObject) widget->onSelectRenderObject(this->widget->renderObjectPtr);
						},
						.child = Row{
							.widget{
								.height = Size::Shrink,
								.alignment = Alignment::CenterLeft,
							},
							.crossAxisAlignment = Row::Alignment::center,
							.children{
								// Dropdown icon
								childrenCount == 0//
									? Child{
										  Row{
											  .widget{
												  .width = 16.f,
												  .height = 16.f,
											  },
										  },
									  }
									: Transform{
										  .translate{-iconOffset, iconOffset},
										  .rotate = iconRotate,
										  .child = FontIcon{
											  .icon = 0xe5cc,
										  },
									  },
								// Element name
								Text{
									.text = getElementName(renderObject),
								},
							},
						},
					},
				},
				// Children
				Column{
					.widget{
						.padding = Padding{}.withLeft(8.f),
					},
					.children = [&]() -> Children {
						Children ret;

						if (!open) return ret;

						auto children = renderObject->getChildren();
						for (const auto &child: children) {
							ret.emplace_back(
								LayoutInspectorItem{
									.renderObjectPtr = child->weak_from_this(),
									.onHoverRenderObject = widget->onHoverRenderObject,
									.onSelectRenderObject = widget->onSelectRenderObject,
								}
							);
						}

						return ret;
					}(),
				},
			},
		};
	}

	struct LayoutInspectorOverlay : RenderObjectWidget {
		struct Value {
			Margin margin;
			Padding padding;
			Rect bounds;

			bool operator==(const Value &other) const = default;
		};

		Key key;
		std::optional<Value> value;

		struct LayoutInspectorOverlayRenderObject : SingleChildRenderObject {
			using InspectorPipeline = Engine::Pipeline<Engine::InspectorQuad::Vertex>;

			std::optional<Value> value;
			std::shared_ptr<InspectorPipeline> pipeline;

			Rect getHitcheckRect() const override {
				return Rect::fromPosSize({}, {});
			}

			void init() override {
				auto *app = getApp();
				pipeline = app->pipelineStore.getPipeline(Store::PipelineProvider<InspectorPipeline>{
					.key = "squiLayoutInspectorOverlayPipeline",
					.provider = [&]() {
						return InspectorPipeline::Args{
							.vertexShader = Engine::Shaders::inspectorQuadvert,
							.fragmentShader = Engine::Shaders::inspectorQuadfrag,
							.instance = app->engine.instance,
						};
					},
				});
			}

			void drawSelf() override {
				if (!value.has_value()) return;
				if (!pipeline) return;
				Engine::InspectorQuad quad{Engine::InspectorQuad::Args{
					.position = value->bounds.getTopLeft(),
					.size = value->bounds.size(),
					.margins = value->margin,
					.paddings = value->padding,
				}};
				pipeline->bind();
				auto [vi, ii] = pipeline->getIndexes();
				pipeline->addData(quad.getData(vi, ii));
			}
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<LayoutInspectorOverlayRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const {
			auto *obj = renderObject->as<LayoutInspectorOverlayRenderObject>();
			if (!obj) return;
			if (obj->value != value) {
				obj->value = value;
				obj->getApp()->needsRedraw = true;
			}
		}

		static Args getArgs() {
			return {};
		}
	};

	struct LayoutInspectorDetails : StatelessWidget {
		// Args
		Key key;
		std::weak_ptr<RenderObject> selectedRenderObject;

		[[nodiscard]] Child build(const Element &) const {
			auto renderObject = selectedRenderObject.lock();
			if (!renderObject) return {};

			return Box{
				.widget{
					.height = Size::Shrink,
					.padding = 16.f,
				},
				.color = Color::black * 0.1f,
				.borderColor = Color::black * 0.8f,
				.borderWidth = BorderWidth::Top(1.f),
				.child = Column{
					.children{
						Text{.text = "Specs:", .fontSize = 20.f},
						Text{
							.text = std::format(
								"Width: {}",
								std::visit(
									utils::overloaded{
										[](float size) -> std::string {
											return std::format("{}px", size);
										},
										[](Size size) -> std::string {
											switch (size) {
												case Size::Expand:
													return "Expand";
												case Size::Shrink:
													return "Shrink";
												case Size::Wrap:
													return "Wrap";
											}
											std::unreachable();
										},
									},
									renderObject->width
								)
							),
						},
						Text{
							.text = std::format(
								"Height: {}",
								std::visit(
									utils::overloaded{
										[](float size) -> std::string {
											return std::format("{}px", size);
										},
										[](Size size) -> std::string {
											switch (size) {
												case Size::Expand:
													return "Expand";
												case Size::Shrink:
													return "Shrink";
												case Size::Wrap:
													return "Wrap";
											}
											std::unreachable();
										},
									},
									renderObject->height
								)
							),
						},
						Text{.text = std::format("Margin: t:{}, r:{}, b:{}, l:{}", renderObject->margin.top, renderObject->margin.right, renderObject->margin.bottom, renderObject->margin.left)},
						Text{.text = std::format("Padding: t:{}, r:{}, b:{}, l:{}", renderObject->padding.top, renderObject->padding.right, renderObject->padding.bottom, renderObject->padding.left)},
						renderObject->alignment.has_value()//
							? Text{.text = std::format("Alignment: {}x{}", renderObject->alignment->horizontal, renderObject->alignment->vertical)}
							: Child{},
						Text{.widget{.margin = Margin{}.withTop(8.f)}, .text = "Sizing:", .fontSize = 20.f},
						Text{
							.text = std::format("Size: {}x{}", renderObject->size.x, renderObject->size.y),
						},
					},
				},
			};
		}
	};

	void LayoutInspector::State::initState() {
		onHoverRenderObject = [this](const std::pair<bool, std::weak_ptr<RenderObject>> &hover) {
			if (hover.second.lock() != hoveredRenderObject.lock()) {
				setState([&]() {
					hoveredRenderObject = hover.second.lock();
				});
			} else if (!hover.first) {
				setState([&]() {
					hoveredRenderObject.reset();
				});
			}
		};
		onSelectRenderObject = [this](const std::weak_ptr<RenderObject> &renderObject) {
			setState([&]() {
				selectedRenderObject = renderObject;
			});
		};
	}

	core::Child LayoutInspector::State::build(const Element &) {
		return Gesture{
			.onUpdate = [this](const Gesture::State &state) {
				if (state.isKeyPressedOrRepeat(GestureKey::f12)) {
					setState([&]() {
						visible = !visible;
					});
				}
			},
			.child = Row{
				.widget{},
				.children{
					Stack{
						.children{
							Wrapper{
								.afterLayout = [this](RenderObject &renderObject) {
									if (visible && contentRenderObject.lock() != renderObject.shared_from_this()) setState([&]() {
										contentRenderObject = renderObject.weak_from_this();
									});
								},
								.child = Navigator{
									.child = widget->child,
								},
							},
							LayoutInspectorOverlay{
								.value = [&]() -> std::optional<LayoutInspectorOverlay::Value> {
									auto renderObject = hoveredRenderObject.lock();
									if (!renderObject) return std::optional<LayoutInspectorOverlay::Value>{};
									return LayoutInspectorOverlay::Value{
										.margin = renderObject->margin,
										.padding = renderObject->padding,
										.bounds = renderObject->getLayoutRect(),
									};
								}(),
							},
						},
					},
					visible//
						? Box{
							  .widget{
								  .width = 400.f,
							  },
							  .color = Color::white * 0.1f,
							  .child = Column{
								  .children{
									  ScrollView{
										  .scrollWidget{
											  .padding = 4.f,
										  },
										  .children{
											  LayoutInspectorItem{
												  .renderObjectPtr = this->contentRenderObject,
												  .onHoverRenderObject = this->onHoverRenderObject,
												  .onSelectRenderObject = this->onSelectRenderObject,
											  },
										  },
									  },
									  LayoutInspectorDetails{
										  .selectedRenderObject = this->selectedRenderObject,
									  },
								  },
							  },
						  }
						: Child(nullptr),
				},
			},
		};
	}
}// namespace squi