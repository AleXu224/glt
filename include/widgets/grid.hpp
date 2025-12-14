#pragma once

#include "core/core.hpp"

namespace squi {
	struct Grid : RenderObjectWidget {
		struct MinSize {
			float value;

            auto operator<=>(const MinSize &) const = default;
		};

		// Args
		Key key;
		Args widget;
		std::variant<int, MinSize> columnCount;
		float spacing = 0.0f;
		Children children;

		struct Element : core::MultiChildRenderObjectElement {
			using core::MultiChildRenderObjectElement::MultiChildRenderObjectElement;

			std::vector<Child> build() override {
				if (auto gridWidget = std::static_pointer_cast<Grid>(widget)) {
					return gridWidget->children;
				}
				return {};
			}
		};

		struct GridRenderObject : core::MultiChildRenderObject {
			GridRenderObject() : core::MultiChildRenderObject() {}

            std::variant<int, MinSize> columnCount;
			float spacing = 0.0f;
            size_t columns = 1;
			std::vector<float> rowHeights{};

			vec2 calculateContentSize(BoxConstraints constraints, bool final) override;
			void positionContentAt(const Rect &newBounds) override;

            [[nodiscard]] size_t computeColumnCount(float availableWidth) const;

			void init() override;
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<GridRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const;
	};
}// namespace squi