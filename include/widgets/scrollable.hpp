#pragma once

#include "core/core.hpp"
#include "misc/scrollViewData.hpp"

namespace squi {
	struct Scrollable : RenderObjectWidget {
		enum class Alignment : uint8_t {
			begin,
			center,
			end
		};

		Key key;
		Args widget;
		Alignment alignment = Alignment::begin;
		Axis direction = Axis::Vertical;
		float spacing = 0.0f;
		float scroll = 0.0f;
		std::shared_ptr<ScrollViewData> controller{std::make_shared<ScrollViewData>()};
		Children children{};

		struct Element : MultiChildRenderObjectElement {
			using MultiChildRenderObjectElement::MultiChildRenderObjectElement;

			std::vector<Child> build() override {
				if (auto scrollableWidget = std::static_pointer_cast<Scrollable>(widget)) {
					return scrollableWidget->children;
				}
				return {};
			};
		};

		struct ScrollableRenderObject : MultiChildRenderObject {
			float scroll = 0;
			float contentMainAxis = 0.f;
			float spacing = 0;
			Alignment alignment = Alignment::begin;
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

		static Args getArgs() {
			return {};
		}
	};
}// namespace squi