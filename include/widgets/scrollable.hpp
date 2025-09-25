#pragma once

#include "core/core.hpp"
#include "misc/scrollViewData.hpp"
#include "widgets/flex.hpp"

namespace squi {
	struct Scrollable : RenderObjectWidget {
		Key key;
		Args widget;
		Flex::Alignment alignment = Flex::Alignment::start;
		Axis direction = Axis::Vertical;
		float spacing = 0.0f;
		float scroll = 0.0f;
		std::shared_ptr<ScrollViewData> controller{std::make_shared<ScrollViewData>()};
		Children children{};

		struct Element : SingleChildRenderObjectElement {
			using SingleChildRenderObjectElement::SingleChildRenderObjectElement;

			Child build() override {
				if (auto scrollableWidget = std::static_pointer_cast<Scrollable>(widget)) {
					return Flex{
						.direction = scrollableWidget->direction,
						.crossAxisAlignment = scrollableWidget->alignment,
						.spacing = scrollableWidget->spacing,
						.children = std::move(scrollableWidget->children),
					};
				}
				return {};
			};
		};

		struct ScrollableRenderObject : SingleChildRenderObject {
			float scroll = 0;
			float contentMainAxis = 0.f;
			Axis direction = Axis::Vertical;
			std::shared_ptr<ScrollViewData> controller;

			void init() override;

			vec2 calculateContentSize(BoxConstraints constraints, bool final) override;
			void afterSizeCalculated() override;

			void positionContentAt(const Rect &newBounds) override;

			void drawContent() override;
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<ScrollableRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const;
	};
}// namespace squi