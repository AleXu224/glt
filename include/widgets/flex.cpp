#include "widgets/flex.hpp"

#include "core/app.hpp"
#include <algorithm>

namespace squi {
	vec2 Flex::FlexRenderObject::calculateContentSize(BoxConstraints constraints, bool final) {
		return this->_calculateContentSize(constraints, final, false);
	}

	[[nodiscard]] vec2 Flex::FlexRenderObject::_calculateContentSize(BoxConstraints constraints, bool final, bool ignoreCrossAxisSize) {
		const auto childCount = children.size();

		float totalMainAxis = 0.0f;
		float maxCrossAxis = 0.0f;
		bool &crossAxisShrink = direction == Axis::Horizontal ? constraints.shrinkHeight : constraints.shrinkWidth;
		bool &mainAxisShrink = direction == Axis::Horizontal ? constraints.shrinkWidth : constraints.shrinkHeight;

		auto &mainAxisMaxSize = direction == Axis::Horizontal ? constraints.maxWidth : constraints.maxHeight;
		// const auto &mainAxisMinSize = direction == Axis::Horizontal ? constraints.minWidth : constraints.minHeight;
		auto &crossAxisMaxSize = direction == Axis::Horizontal ? constraints.maxHeight : constraints.maxWidth;
		const auto &crossAxisMinSize = direction == Axis::Horizontal ? constraints.minHeight : constraints.minWidth;

		std::vector<RenderObjectPtr> expandedChildren{};

		if (!ignoreCrossAxisSize && crossAxisShrink) {
			const auto size = _calculateContentSize(constraints, final, true).x;
			crossAxisMaxSize = std::clamp(size, crossAxisMinSize, crossAxisMaxSize);
			crossAxisShrink = false;
		}

		float spacingOffset = spacing * (static_cast<float>(childCount) - 1.f);
		spacingOffset = (std::max) (0.0f, spacingOffset);

		mainAxisMaxSize -= spacingOffset;

		for (auto &child: children) {
			if (!child) continue;

			auto sizing = child->getSizing(direction);

			if (!mainAxisShrink && sizing == Sizing::Expand) {
				expandedChildren.emplace_back(child);
			} else {
				BoxConstraints childConstraints = constraints;
				if (direction == Axis::Horizontal) {
					childConstraints.minWidth = 0.0f;
					childConstraints.maxWidth -= totalMainAxis;
				} else {
					childConstraints.minHeight = 0.0f;
					childConstraints.maxHeight -= totalMainAxis;
				}
				const auto childSize = child->calculateSize(childConstraints, final);
				totalMainAxis += direction == Axis::Horizontal ? childSize.x : childSize.y;
				maxCrossAxis = std::max(maxCrossAxis, direction == Axis::Horizontal ? childSize.y : childSize.x);
			}
		}

		if (!expandedChildren.empty() && mainAxisMaxSize > totalMainAxis) {
			BoxConstraints childConstraints = constraints;
			float newMainAxisMaxSize = std::max(0.f, mainAxisMaxSize - totalMainAxis) / static_cast<float>(expandedChildren.size());
			if (direction == Axis::Horizontal) {
				childConstraints.minWidth = 0.0f;
				childConstraints.maxWidth = newMainAxisMaxSize;
			} else {
				childConstraints.minHeight = 0.0f;
				childConstraints.maxHeight = newMainAxisMaxSize;
			}
			for (auto &child: expandedChildren) {
				const auto childSize = child->calculateSize(childConstraints, final);
				totalMainAxis += direction == Axis::Horizontal ? childSize.x : childSize.y;
				maxCrossAxis = std::max(maxCrossAxis, direction == Axis::Horizontal ? childSize.y : childSize.x);
			}
		}

		if (direction == Axis::Horizontal) {
			return {
				totalMainAxis + spacingOffset,
				maxCrossAxis,
			};
		}
		return {
			maxCrossAxis,
			totalMainAxis + spacingOffset,
		};
	}

	void Flex::FlexRenderObject::positionContentAt(const Rect &newBounds) {
		// auto mainAxisSize = direction == Axis::Horizontal ? newBounds.width() : newBounds.height();
		auto crossAxisSize = direction == Axis::Horizontal ? newBounds.height() : newBounds.width();
		auto cursor = newBounds.getTopLeft();
		const auto initialCrossAxis = direction == Axis::Horizontal ? cursor.y : cursor.x;

		auto &cursorMainAxis = direction == Axis::Horizontal ? cursor.x : cursor.y;
		auto &cursorCrossAxis = direction == Axis::Horizontal ? cursor.y : cursor.x;

		for (auto &child: children) {
			if (!child) continue;
			auto childSize = Rect::fromPosSize({}, child->getLayoutRect().size());
			const auto childCrossAxisSize = direction == Axis::Horizontal ? childSize.height() : childSize.width();

			switch (crossAxisAlignment) {
				case Alignment::start:
					cursorCrossAxis = initialCrossAxis;
					break;
				case Alignment::center:
					cursorCrossAxis = initialCrossAxis + std::round((crossAxisSize - childCrossAxisSize) / 2.0f);
					break;
				case Alignment::end:
					cursorCrossAxis = initialCrossAxis + crossAxisSize - childCrossAxisSize;
					break;
			}

			child->positionAt(childSize.offset(cursor));
			cursorMainAxis += (direction == Axis::Horizontal ? childSize.width() : childSize.height()) + spacing;
		}
	}

	void Flex::FlexRenderObject::init() {
		this->getWidgetAs<Flex>()->updateRenderObject(this);
	}

	void Flex::updateRenderObject(RenderObject *renderObject) const {
		// Update render object properties here
		if (auto *flexRenderObject = dynamic_cast<FlexRenderObject *>(renderObject)) {
			auto *app = flexRenderObject->getApp();
			if (flexRenderObject->direction != direction) {
				flexRenderObject->direction = direction;
				app->needsRelayout = true;
			}

			if (flexRenderObject->crossAxisAlignment != crossAxisAlignment) {
				flexRenderObject->crossAxisAlignment = crossAxisAlignment;
				app->needsReposition = true;
			}

			if (flexRenderObject->spacing != spacing) {
				flexRenderObject->spacing = spacing;
				app->needsRelayout = true;
			}
		}
	}
}// namespace squi