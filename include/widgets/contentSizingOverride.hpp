#pragma once

#include <core/core.hpp>

namespace squi {
	struct ContentSizingOverride : core::RenderObjectWidget {
		Key key;
		std::optional<Sizing> widthSizing;
		std::optional<Sizing> heightSizing;
		Child child;

		struct Element : core::SingleChildRenderObjectElement {
			Element(const RenderObjectWidgetPtr &widget) : SingleChildRenderObjectElement(widget) {}

			Child build() override {
				if (auto contentSizingWidget = std::static_pointer_cast<ContentSizingOverride>(widget)) {
					return contentSizingWidget->child;
				}
				return nullptr;
			}
		};

		struct VisibilityRenderObject : core::SingleChildRenderObject {
			std::optional<Sizing> widthSizing;
            std::optional<Sizing> heightSizing;

			void init() override {
				this->getWidgetAs<ContentSizingOverride>()->updateRenderObject(this);
			}

			Sizing getContentSizing(Axis axis) const override;
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<VisibilityRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const;

		[[nodiscard]] Args getArgs() const {
            return {
                .width = Size::Wrap,
                .height = Size::Wrap,
            };
		}
	};
}// namespace squi