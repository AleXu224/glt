#include "offset.hpp"
#include "core/app.hpp"

namespace squi {
	void Offset::OffsetRenderObject::positionContentAt(const Rect &newBounds) {
		auto *widget = this->getWidgetAs<Offset>();
		if (widget->calculateContentBounds) {
			auto contentBounds = widget->calculateContentBounds(newBounds, *this);
			lastBounds = newBounds;
			lastCalculatedBounds = contentBounds;
			SingleChildRenderObject::positionContentAt(contentBounds);
			pos = contentBounds.getTopLeft();
		} else {
			SingleChildRenderObject::positionContentAt(newBounds);
		}
	}

	void Offset::updateRenderObject(RenderObject *renderObject) const {
		if (auto *offsetRenderObject = dynamic_cast<OffsetRenderObject *>(renderObject)) {
			if (offsetRenderObject->lastCalculatedBounds != calculateContentBounds(offsetRenderObject->lastBounds, *offsetRenderObject)) {
				auto *app = renderObject->getApp();
				app->needsReposition = true;
			}
		}
	}
}// namespace squi