#pragma once

#include "core/core.hpp"

namespace squi {
	struct LayoutBuilder : RenderObjectWidget {
		Key key;
		Args widget;
		std::function<Child(BoxConstraints)> builder;

		struct Element : SingleChildRenderObjectElement {
			using SingleChildRenderObjectElement::SingleChildRenderObjectElement;
			BoxConstraints parentConstraints;

			Child build() override {
				if (auto layoutBuilderWidget = std::static_pointer_cast<LayoutBuilder>(widget)) {
					return layoutBuilderWidget->builder(parentConstraints);
				}
				return {};
			}
		};

		struct LayoutBuilderRenderObject : SingleChildRenderObject {
			vec2 calculateContentSize(BoxConstraints constraints, bool final) override {
				if (final) {
					dynamic_cast<Element &>(*element).parentConstraints = constraints;
					element->rebuild();
					return SingleChildRenderObject::calculateContentSize(constraints, final);
				}
				return {};
			}
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<LayoutBuilderRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const {
			// Update render object properties here
		}

		Args getArgs() const {
			auto ret = widget;
			ret.width = ret.width.value_or(Size::Wrap);
			ret.height = ret.height.value_or(Size::Wrap);
			return ret;
		}
	};
}// namespace squi