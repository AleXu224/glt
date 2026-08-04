#include "visibility.hpp"

namespace squi {
	void Visibility::VisibilityRenderObject::update() {
		if (!visible) return;
		SingleChildRenderObject::update();
	}

	bool Visibility::VisibilityRenderObject::hitTest(const vec2 &pos, std::vector<HitEntry> &path) {
		if (!visible) return false;
		return SingleChildRenderObject::hitTest(pos, path);
	}

	vec2 Visibility::VisibilityRenderObject::calculateContentSize(BoxConstraints constraints, bool final) {
		if (!visible) return vec2{0.f, 0.f};
		return SingleChildRenderObject::calculateContentSize(constraints, final);
	}

	void Visibility::VisibilityRenderObject::positionContentAt(const Rect &newBounds) {
		if (!visible) return;
		SingleChildRenderObject::positionContentAt(newBounds);
	}

	void Visibility::VisibilityRenderObject::drawContent() {
		if (!visible) return;
		SingleChildRenderObject::drawContent();
	}

	void Visibility::updateRenderObject(RenderObject *renderObject) const {
		if (auto visibilityRenderObject = dynamic_cast<VisibilityRenderObject *>(renderObject)) {
			if (visibilityRenderObject->visible != visible) {
				visibilityRenderObject->visible = visible;
				visibilityRenderObject->element->markNeedsRelayout();
			}
		}
	}
}// namespace squi