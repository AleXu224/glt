#include "layoutInspector.hpp"
#include "compiledShaders/inspectorQuadfrag.hpp"
#include "compiledShaders/inspectorQuadvert.hpp"
#include "inspectorQuad.hpp"
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
						},
						.content = Row{
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
								.child = widget->child,
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
							  .child = ScrollView{
								  .scrollWidget{
									  .padding = 4.f,
								  },
								  .children{
									  LayoutInspectorItem{
										  .renderObjectPtr = this->contentRenderObject,
										  .onHoverRenderObject = this->onHoverRenderObject,
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