#include "widgets/scrollable.hpp"
#include "core/app.hpp"

namespace squi {
	void Scrollable::updateRenderObject(RenderObject *renderObject) const {
		if (auto *scrollableRenderObject = dynamic_cast<ScrollableRenderObject *>(renderObject)) {
			auto *app = renderObject->getApp();

			if (scroll != scrollableRenderObject->scroll) {
				scrollableRenderObject->scroll = scroll;
				app->needsReposition = true;
			}

			if (direction != scrollableRenderObject->direction) {
				scrollableRenderObject->direction = direction;
				app->needsRelayout = true;
			}

			scrollableRenderObject->controller = controller;
		}
	}

	void Scrollable::ScrollableRenderObject::init() {
		this->getWidgetAs<Scrollable>()->updateRenderObject(this);
	}

	void Scrollable::ScrollableRenderObject::update() {
		auto *app = getApp();
		if (app) {
			app->inputState.g_activeArea.emplace_back(getRect());
		}
		SingleChildRenderObject::update();
		if (app) {
			app->inputState.g_activeArea.pop_back();
		}
	}

	vec2 Scrollable::ScrollableRenderObject::calculateContentSize(BoxConstraints constraints, bool final) {
		auto childConstraints = constraints;
		float totalMainAxis = 0.f;
		float maxCrossAxis = 0.f;

		switch (direction) {
			case Axis::Vertical:
				childConstraints.shrinkHeight = true;
				childConstraints.maxHeight = std::numeric_limits<float>::max();
				childConstraints.minHeight = 0.f;
				break;
			case Axis::Horizontal:
				childConstraints.shrinkWidth = true;
				childConstraints.maxWidth = std::numeric_limits<float>::max();
				childConstraints.minWidth = 0.f;
				break;
		}

		if (child) {
			const auto size = child->calculateSize(childConstraints, final);
			switch (direction) {
				case Axis::Vertical:
					totalMainAxis += size.y;
					maxCrossAxis = std::max(maxCrossAxis, size.x);
					break;
				case Axis::Horizontal:
					totalMainAxis += size.x;
					maxCrossAxis = std::max(maxCrossAxis, size.y);
					break;
			}
		}

		contentMainAxis = totalMainAxis;

		switch (direction) {
			case Axis::Vertical:
				return {maxCrossAxis, totalMainAxis};
			case Axis::Horizontal:
				return {totalMainAxis, maxCrossAxis};
		}
		std::unreachable();
	}

	void Scrollable::ScrollableRenderObject::afterSizeCalculated() {
		auto viewMainAxis = 0.f;

		switch (direction) {
			case Axis::Vertical:
				viewMainAxis = getContentRect().height();
				break;
			case Axis::Horizontal:
				viewMainAxis = getContentRect().width();
				break;
		}

		controller->viewMainAxis = viewMainAxis;
		controller->contentMainAxis = contentMainAxis;
	}

	void Scrollable::ScrollableRenderObject::positionContentAt(const Rect &newBounds) {
		const auto childPos = newBounds.posFromAlignment(::squi::Alignment::TopLeft, size);

		if (!child) return;
		switch (direction) {
			case Axis::Vertical: {
				child->positionAt(
					Rect::fromPosSize(
						childPos.withYOffset(-std::round(scroll)),
						child->getLayoutRect().size()
					)
				);
				break;
			}
			case Axis::Horizontal: {
				child->positionAt(
					Rect::fromPosSize(
						childPos.withXOffset(-std::round(scroll)),
						child->getLayoutRect().size()
					)
				);
				break;
			}
		}
	}

	void Scrollable::ScrollableRenderObject::drawContent() {
		if (!child) return;
		auto *app = this->getApp();
		auto &instance = app->engine.instance;
		instance.pushScissor(getRect());
		child->draw();
		instance.popScissor();
	}
}// namespace squi