#pragma once

#include "core/core.hpp"

namespace squi {
	struct Offset : RenderObjectWidget {
		Key key;
		std::function<Rect(const Rect &, const SingleChildRenderObject &)> calculateContentBounds;
		Child child;

		struct Element : SingleChildRenderObjectElement {
			using SingleChildRenderObjectElement::SingleChildRenderObjectElement;

			Child build() override {
				if (auto offsetWidget = std::static_pointer_cast<Offset>(widget)) {
					return offsetWidget->child;
				}
				return {};
			};
		};

		struct OffsetRenderObject : SingleChildRenderObject {
			void positionContentAt(const Rect &newBounds) override {
				auto *widget = this->getWidgetAs<Offset>();
				if (widget->calculateContentBounds) {
					auto contentBounds = widget->calculateContentBounds(newBounds, *this);
					SingleChildRenderObject::positionContentAt(contentBounds);
				} else {
					SingleChildRenderObject::positionContentAt(newBounds);
				}
			}
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<OffsetRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const {}

		static Args getArgs() {
			return {
				.width = Size::Wrap,
				.height = Size::Wrap,
			};
		}
	};
}// namespace squi