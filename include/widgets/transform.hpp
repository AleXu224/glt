#pragma once

#include "core/core.hpp"

namespace squi {
	struct Transform : RenderObjectWidget {
		Key key;
		vec2 origin{0.5f, 0.5f};
		vec2 translate{0.f, 0.f};
		vec2 scale{1.f, 1.f};
		float rotate = 0.f;
		Child child;

		struct Element : SingleChildRenderObjectElement {
			using SingleChildRenderObjectElement::SingleChildRenderObjectElement;

			Child build() override {
				return getWidgetAs<Transform>()->child;
			}
		};

		struct TransformRenderObject : SingleChildRenderObject {
			vec2 origin{0.5f, 0.5f};
			vec2 translate{0.f, 0.f};
			vec2 scale{1.f, 1.f};
			float rotate = 0.f;

			void drawContent() override;

			void init() override {
				this->getWidgetAs<Transform>()->updateRenderObject(this);
			}
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<TransformRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const;

		static Args getArgs() {
			return {
				.width = Size::Wrap,
				.height = Size::Wrap,
			};
		}
	};
}// namespace squi