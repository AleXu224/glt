#include "widgets/scrollable.hpp"
#include "core/app.hpp"

namespace squi {
	void Scrollable::updateRenderObject(RenderObject *renderObject) const {
		if (auto *scrollableRenderObject = dynamic_cast<ScrollableRenderObject *>(renderObject)) {
			auto *app = renderObject->getApp();

			if (spacing != scrollableRenderObject->spacing) {
				scrollableRenderObject->spacing = spacing;
				app->needsReposition = true;
			}

			if (direction != scrollableRenderObject->direction) {
				scrollableRenderObject->direction = direction;
				app->needsRelayout = true;
			}

			scrollableRenderObject->controller = controller;
			scrollableRenderObject->onScroll = onScroll;
		}
	}

	void Scrollable::ScrollableRenderObject::init() {
		this->getWidgetAs<Scrollable>()->updateRenderObject(this);
	}
}// namespace squi