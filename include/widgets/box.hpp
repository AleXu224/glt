#pragma once

#include "borderRadius.hpp"
#include "borderWidth.hpp"
#include "color.hpp"
#include "core/core.hpp"


namespace squi {
	struct BoxData;
	struct Box : core::RenderObjectWidget {
		enum class BorderPosition : uint8_t {
			inset,
			outset,
		};
		Args widget{};

		Color color{0xFFFFFFFF};
		Color borderColor{0x000000FF};
		BorderWidth borderWidth{0.0f};
		BorderRadius borderRadius{0.0f};
		BorderPosition borderPosition{BorderPosition::inset};
		bool shouldClipContent = true;
		Child child;

		struct Element : core::SingleChildRenderObjectElement {
			Element(const RenderObjectWidgetPtr &widget) : SingleChildRenderObjectElement(widget) {}

			Child build() override {
				if (auto boxWidget = std::static_pointer_cast<Box>(widget)) {
					return boxWidget->child;
				}
				return nullptr;
			}
		};

		struct BoxRenderObject : core::SingleChildRenderObject {
			std::unique_ptr<BoxData> data;

			bool shouldClipContent = true;

			BoxRenderObject();

			void init() override;
			void drawSelf() override;
		};

		std::shared_ptr<RenderObject> createRenderObject() const;

		void updateRenderObject(RenderObject *renderObject) const;
	};
}// namespace squi