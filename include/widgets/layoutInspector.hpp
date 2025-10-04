#pragma once

#include "button.hpp"
#include "column.hpp"
#include "core/core.hpp"
#include "fontIcon.hpp"
#include "transform.hpp"
#include "widgets/box.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/row.hpp"
#include "widgets/scrollview.hpp"
#include "widgets/text.hpp"
#include "wrapper.hpp"
#include <GLFW/glfw3.h>

namespace squi {
	struct LayoutInspectorItem : StatefulWidget {
		// Args
		Key key;
		std::weak_ptr<RenderObject> renderObjectPtr;

		struct State final : WidgetState<LayoutInspectorItem> {
			bool open = false;
			Animated<float> iconRotate{.from = 0.f};
			Animated<float> iconOffset{.from = 0.f};

			void initState() override {
				iconRotate.mount(this);
				iconOffset.mount(this);
			}

			[[nodiscard]] static std::string getElementName(const RenderObjectPtr &renderObject) {
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

			Child build(const Element &) override {
				auto renderObject = widget->renderObjectPtr.lock();
				if (!renderObject) return nullptr;

				return Column{
					.children{
						Button{
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
								.children{
									Transform{
										.translate{-iconOffset, iconOffset},
										.rotate = iconRotate,
										.child = FontIcon{
											.icon = 0xe5cc,
										},
									},
									Text{
										.text = getElementName(renderObject),
									},
								},
							},
						},
						Column{
							.widget{
								.padding = Padding{}.withLeft(8.f),
							},
							.children = [&]() -> Children {
								Children ret;

								if (!open) return ret;

								renderObject->iterateChildren([&](RenderObject *childRenderObject) {
									ret.emplace_back(
										LayoutInspectorItem{
											.renderObjectPtr = childRenderObject->weak_from_this(),
										}
									);
								});

								return ret;
							}(),
						},
					},
				};
			}
		};
	};


	struct LayoutInspector : StatefulWidget {
		// Args
		Key key;
		Child child;

		struct State : WidgetState<LayoutInspector> {
			std::weak_ptr<RenderObject> contentRenderObject;
			bool visible = false;

			Child build(const Element &) override {
				return Gesture{
					.onUpdate = [this](const Gesture::State &state) {
						if (state.isKeyPressedOrRepeat(GLFW_KEY_F12)) {
							setState([&]() {
								visible = !visible;
							});
						}
					},
					.child = Row{
						.widget{},
						.children{
							Wrapper{
								.afterLayout = [this](RenderObject &renderObject) {
									if (visible && contentRenderObject.lock() != renderObject.shared_from_this()) setState([&]() {
										contentRenderObject = renderObject.weak_from_this();
									});
								},
								.child = widget->child,
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
											  LayoutInspectorItem{.renderObjectPtr = this->contentRenderObject},
										  },
									  },
								  }
								: Child(nullptr),
						},
					},
				};
			}
		};
	};
}// namespace squi