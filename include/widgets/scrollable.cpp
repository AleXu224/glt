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

		for (auto &child: children) {
			if (!child) continue;

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
		float totalSpacing = spacing * (static_cast<float>(children.size()) - 1.f);
		totalSpacing = std::max(totalSpacing, 0.f);
		contentMainAxis = totalMainAxis + totalSpacing;

		switch (direction) {
			case Axis::Vertical:
				return {maxCrossAxis, contentMainAxis};
			case Axis::Horizontal:
				return {contentMainAxis, maxCrossAxis};
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
		float cursor = 0.f;
		const auto crossAxisWidth = [&]() {
			switch (direction) {
				case Axis::Vertical:
					return getContentRect().width();
				case Axis::Horizontal:
					return getContentRect().height();
			}
			std::unreachable();
		}();

		const auto crossAxisOffsetFactor = [&]() {
			switch (alignment) {
				case Alignment::begin:
					return 0.f;
				case Alignment::center:
					return 0.5f;
				case Alignment::end:
					return 1.f;
			}
			std::unreachable();
		}();

		for (auto &child: children) {
			if (!child) continue;
			switch (direction) {
				case Axis::Vertical: {
					auto offset = (crossAxisWidth - child->getLayoutRect().width()) * crossAxisOffsetFactor;
					child->positionAt(
						Rect::fromPosSize(
							childPos.withYOffset(-std::round(scroll - cursor)).withXOffset(offset),
							child->getLayoutRect().size()
						)
					);
					cursor += child->getLayoutRect().height() + spacing;
					break;
				}
				case Axis::Horizontal: {
					auto offset = (crossAxisWidth - child->getLayoutRect().height()) * crossAxisOffsetFactor;
					child->positionAt(
						Rect::fromPosSize(
							childPos.withXOffset(-std::round(scroll - cursor)).withYOffset(offset),
							child->getLayoutRect().size()
						)
					);
					cursor += child->getLayoutRect().width() + spacing;
					break;
				}
			}
		}
	}

	void Scrollable::ScrollableRenderObject::drawContent() {
		auto *app = this->getApp();
		auto &instance = app->engine.instance;
		instance.pushScissor(getRect());
		for (auto &child: children) {
			if (!child) continue;


			child->draw();
		}
		instance.popScissor();
	}
}// namespace squi