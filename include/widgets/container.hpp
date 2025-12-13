#pragma once

#include "core/core.hpp"

namespace squi {
	struct Container : core::RenderObjectWidget {
		enum class BorderPosition : uint8_t {
			inset,
			outset,
		};
		Key key;
		Args widget{};
		bool shouldClipContent = true;
		Child child;

		struct Element : core::SingleChildRenderObjectElement {
			Element(const RenderObjectWidgetPtr &widget) : SingleChildRenderObjectElement(widget) {}

			Child build() override {
				if (auto containerWidget = std::static_pointer_cast<Container>(widget)) {
					return containerWidget->child;
				}
				return nullptr;
			}
		};

		struct ContainerRenderObject : core::SingleChildRenderObject {
			bool shouldClipContent = true;

			void init() override;
			void drawContent() override;
		};

		static std::shared_ptr<RenderObject> createRenderObject();

		void updateRenderObject(RenderObject *renderObject) const;
	};
}// namespace squi