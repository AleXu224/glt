#pragma once

#include "core/core.hpp"
#include "widgets/box.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/row.hpp"
#include "widgets/scrollview.hpp"
#include "widgets/text.hpp"
#include <GLFW/glfw3.h>

namespace squi {
	struct LayoutInspectorItem : StatefulWidget {
		// Args
		Key key;
		Element *element;


		struct State final : WidgetState<LayoutInspectorItem> {
			[[nodiscard]] std::string getElementName() const {
				std::string ret = typeid(*widget->element->widget).name();
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
				return Box{
					.widget{
						.padding = 8.f,
					},
					.color = Color::white * 0.1f,
					.borderRadius = 4.f,
					.child = Text{
						.text = getElementName(),
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
							widget->child,
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
										  .children{LayoutInspectorItem{.element = this->element}},
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