#include "grid.hpp"
#include "core/app.hpp"
#include <numeric>

namespace squi {
	[[nodiscard]] size_t Grid::GridRenderObject::computeColumnCount(float availableWidth) const {
		return std::visit(
			[&](const auto &val) -> size_t {
				using T = std::decay_t<decltype(val)>;
				if constexpr (std::is_same_v<T, int>) {
					return val;
				} else if constexpr (std::is_same_v<T, MinSize>) {
					const float &minWidth = val.value;
					return std::max(
						static_cast<size_t>(std::floor(availableWidth / minWidth)),
						static_cast<size_t>(1)
					);
				} else {
					static_assert(false, "someone messed up the types");
				}
			},
			columnCount
		);
	}

	vec2 Grid::GridRenderObject::calculateContentSize(BoxConstraints constraints, bool final) {
		const auto columns = computeColumnCount(constraints.shrinkWidth ? 0.f : constraints.maxWidth);

		std::vector<float> maxHeights{};
		float maxWidth = 0.f;
		float totalHorizontalSpacing = (static_cast<float>(columns) - 1.f) * spacing;
		totalHorizontalSpacing = std::max(totalHorizontalSpacing, 0.f);
		auto newMax = vec2{
			(constraints.maxWidth - totalHorizontalSpacing) / static_cast<float>(columns),
			constraints.maxHeight,
		};
		auto newMin = vec2{
			std::max((constraints.minWidth - totalHorizontalSpacing) / static_cast<float>(columns), 0.f),
			0.f,
		};
		for (const auto &chunk: children | std::views::chunk(columns)) {
			float maxHeight = 0.f;
			for (const auto &child: chunk) {
				auto size = child->calculateSize(BoxConstraints{
					.minWidth = newMin.x,
					.maxWidth = newMax.x,
					.minHeight = newMin.y,
					.maxHeight = newMax.y,
					.shrinkWidth = false,
					.shrinkHeight = true,
				});
				maxWidth = std::max(maxWidth, size.x);
				maxHeight = std::max(maxHeight, size.y);
			}
			maxHeights.emplace_back(maxHeight);
		}

		auto maxSize = vec2{maxWidth, 0.f};
		auto minSize = vec2{0.f};
		if (!constraints.shrinkWidth) {
			maxSize.x = std::max(maxSize.x, newMax.x);
		}
		for (const auto &[chunk, maxHeight]: std::views::zip(children | std::views::chunk(columns), maxHeights)) {
			maxSize.y = maxHeight;
			for (const auto &child: chunk) {
				// child->layout(maxSize, minSize, {false, false}, final);
				child->calculateSize(
					BoxConstraints{
						.minWidth = minSize.x,
						.maxWidth = maxSize.x,
						.minHeight = minSize.y,
						.maxHeight = maxSize.y,
						.shrinkWidth = false,
						.shrinkHeight = true,
					},
					final
				);
			}
		}

		size_t rows = (children.size() / columns) + (children.size() % columns == 0 ? 0 : 1);
		rows = std::max(rows, static_cast<size_t>(1));
		float totalVerticalSpacing = (static_cast<float>(rows) - 1.f) * spacing;
		totalVerticalSpacing = std::max(totalVerticalSpacing, 0.f);

		if (final) {
			this->columns = columns;
			this->rowHeights = maxHeights;
		}

		return {
			(maxWidth * static_cast<float>(std::min(columns, children.size()))) + totalHorizontalSpacing,
			std::accumulate(maxHeights.begin(), maxHeights.end(), 0.f) + totalVerticalSpacing
		};
	}
	void Grid::GridRenderObject::positionContentAt(const Rect &newBounds) {
		auto topLeft = newBounds.getTopLeft();

		float yCursor = 0.f;
		for (auto [childrenChunk, maxHeight]: std::views::zip(children | std::views::chunk(columns), rowHeights)) {
			float xCursor = 0.f;
			for (auto &child: childrenChunk) {
				auto childSize = child->getLayoutRect().size();
				child->positionAt(Rect::fromPosSize(topLeft + vec2{xCursor, yCursor}, childSize));
				xCursor += childSize.x + spacing;
			}
			yCursor += maxHeight + spacing;
		}
	}

	void Grid::GridRenderObject::init() {
		this->getWidgetAs<Grid>()->updateRenderObject(this);
	}

	void Grid::updateRenderObject(RenderObject *renderObject) const {
		// Update render object properties here
		if (auto *gridRenderObject = dynamic_cast<GridRenderObject *>(renderObject)) {
			auto *app = gridRenderObject->getApp();
			if (gridRenderObject->columnCount != columnCount) {
				gridRenderObject->columnCount = columnCount;
				app->needsRelayout = true;
			}

			if (gridRenderObject->spacing != spacing) {
				gridRenderObject->spacing = spacing;
				app->needsRelayout = true;
			}
		}
	}
}// namespace squi