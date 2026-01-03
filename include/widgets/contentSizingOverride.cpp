#include "contentSizingOverride.hpp"
#include "core/app.hpp"

namespace squi {
	void ContentSizingOverride::updateRenderObject(RenderObject *renderObject) const {
		if (auto *contentSizingRenderObject = dynamic_cast<VisibilityRenderObject *>(renderObject)) {
			auto *app = renderObject->getApp();

			if (widthSizing != contentSizingRenderObject->widthSizing) {
				contentSizingRenderObject->widthSizing = widthSizing;
				app->needsRelayout = true;
			}

			if (heightSizing != contentSizingRenderObject->heightSizing) {
				contentSizingRenderObject->heightSizing = heightSizing;
				app->needsRelayout = true;
			}
		}
	}
	core::Sizing ContentSizingOverride::VisibilityRenderObject::getContentSizing(Axis axis) const {
		if (axis == Axis::Horizontal && widthSizing) {
			return *widthSizing;
		}
		if (axis == Axis::Vertical && heightSizing) {
			return *heightSizing;
		}
		return SingleChildRenderObject::getContentSizing(axis);
	}
}// namespace squi
