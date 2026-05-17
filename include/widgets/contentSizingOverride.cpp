#include "contentSizingOverride.hpp"

namespace squi {
	void ContentSizingOverride::updateRenderObject(RenderObject *renderObject) const {
		if (auto *contentSizingRenderObject = dynamic_cast<VisibilityRenderObject *>(renderObject)) {
			if (widthSizing != contentSizingRenderObject->widthSizing) {
				contentSizingRenderObject->widthSizing = widthSizing;
				contentSizingRenderObject->element->markNeedsRelayout();
			}

			if (heightSizing != contentSizingRenderObject->heightSizing) {
				contentSizingRenderObject->heightSizing = heightSizing;
				contentSizingRenderObject->element->markNeedsRelayout();
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
