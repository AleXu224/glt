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

			void update() override;
			vec2 calculateContentSize(BoxConstraints constraints, bool final) override;
			void positionContentAt(const Rect &newBounds) override;
			void drawContent() override;
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<VisibilityRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const;

		[[nodiscard]] Args getArgs() const {
			auto ret = widget;
			ret.width = ret.width.value_or(Size::Wrap);
			ret.height = ret.height.value_or(Size::Wrap);
			return ret;
		}
	};
}// namespace squi