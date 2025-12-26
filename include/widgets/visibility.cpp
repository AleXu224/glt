#include "visibility.hpp"
#include "core/app.hpp"

namespace squi {
	void Visibility::updateRenderObject(RenderObject *renderObject) const {
		if (auto visibilityRenderObject = dynamic_cast<VisibilityRenderObject *>(renderObject)) {
			if (visibilityRenderObject->visible != visible) {
				visibilityRenderObject->visible = visible;
				renderObject->getApp()->needsRedraw = true;
			}
		}
	}
}// namespace squi