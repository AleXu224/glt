#pragma once

#include <core/core.hpp>

namespace squi {
	struct Visibility : core::RenderObjectWidget {
		Key key;
		Args widget{};
		bool visible = true;
		Child child;

		struct Element : core::SingleChildRenderObjectElement {
			Element(const RenderObjectWidgetPtr &widget) : SingleChildRenderObjectElement(widget) {}

			Child build() override {
				if (auto boxWidget = std::static_pointer_cast<Visibility>(widget)) {
					return boxWidget->child;
				}
				return nullptr;
			}
		};

		struct VisibilityRenderObject : core::SingleChildRenderObject {
			bool visible = true;

			void init() override {
				this->getWidgetAs<Visibility>()->updateRenderObject(this);
			}

			void update() override {
				if (!visible) return;
				SingleChildRenderObject::update();
			}

			vec2 calculateContentSize(BoxConstraints constraints, bool final) override {
				if (!visible) return vec2{0.f, 0.f};
				return SingleChildRenderObject::calculateContentSize(constraints, final);
			}

			void positionContentAt(const Rect &newBounds) override {
				if (!visible) return;
				SingleChildRenderObject::positionContentAt(newBounds);
			}

			void drawContent() override {
				if (!visible) return;
				SingleChildRenderObject::drawContent();
			}
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<VisibilityRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const;
	};
}// namespace squi