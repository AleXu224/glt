#pragma once

#include "core/core.hpp"

namespace squi {
	struct Stack : RenderObjectWidget {
		Key key;
		Args widget;
		Children children;

		struct Element : MultiChildRenderObjectElement {
			Element(const RenderObjectWidgetPtr &widget) : MultiChildRenderObjectElement(widget) {}

			Children build() override {
				auto stackWidget = std::static_pointer_cast<Stack>(this->widget);
				return stackWidget->children;
			}
		};

		struct StackRenderObject : MultiChildRenderObject {
			StackRenderObject() : MultiChildRenderObject() {}

			void iterateChildren(const std::function<void(RenderObject *)> &callback) override;
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<StackRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const {
			// Update render object properties here
		}
	};
};// namespace squi