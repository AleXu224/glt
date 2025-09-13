#pragma once

#include "core/core.hpp"

namespace squi {
	struct Flex : core::RenderObjectWidget {
		enum class Alignment : uint8_t {
			start,
			center,
			end,
		};

		// Args
		Key key;
		Args widget;
		Axis direction = core::Axis::Horizontal;
		Alignment crossAxisAlignment = Alignment::start;
		float spacing = 0.0f;
		std::vector<Child> children{};

		struct Element : core::MultiChildRenderObjectElement {
			using core::MultiChildRenderObjectElement::MultiChildRenderObjectElement;

			std::vector<Child> build() override {
				if (auto flexWidget = std::static_pointer_cast<Flex>(widget)) {
					return flexWidget->children;
				}
				return {};
			}
		};

		struct FlexRenderObject : core::MultiChildRenderObject {
			FlexRenderObject() : core::MultiChildRenderObject() {}

			core::Axis direction = core::Axis::Horizontal;
			Alignment crossAxisAlignment = Alignment::start;
			float spacing = 0.0f;

			vec2 calculateContentSize(BoxConstraints constraints, bool final) override;
			[[nodiscard]] vec2 _calculateContentSize(BoxConstraints constraints, bool final, bool ignoreCrossAxisSize = false);
			void positionContentAt(const Rect &newBounds) override;

			void init() override;
		};

		static RenderObjectPtr createRenderObject() {
			return std::make_shared<FlexRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const;
	};
}// namespace squi