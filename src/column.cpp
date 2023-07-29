#include "column.hpp"
#include "child.hpp"
#include "renderer.hpp"
#include "widget.hpp"
#include <iterator>
#include <numeric>
#include <tuple>

using namespace squi;

Column::Impl::Impl(const Column &args)
	: Widget(args.widget, Widget::Flags::Default()),
	  alignment(args.alignment), spacing(args.spacing) {
	setChildren(args.children);
}

void Column::Impl::layoutChildren(vec2 &maxSize, vec2 &minSize) {
	auto &children = getChildren();

	float totalHeight = 0.0f;
	float maxWidth = 0.0f;

	std::vector<Child> expandedChildren{};

	const vec2 maxChildSize = maxSize + state.padding.getSizeOffset();

	for (auto &child: children) {
		if (!child) continue;

		auto &childState = child->state;
		childState.parent = this;
		childState.root = state.root;

		if (childState.sizeMode.height.index() == 1 && std::get<1>(childState.sizeMode.height) == Size::Expand) {
			expandedChildren.emplace_back(child);
		} else {
			const auto childSize = child->layout(maxChildSize);
			totalHeight += childSize.y;
			if (childState.sizeMode.width.index() != 1 || std::get<1>(childState.sizeMode.width) != Size::Expand) {
				maxWidth = (std::max)(maxWidth, childSize.x);
			}
		}
	}

	float spacingOffset = spacing * static_cast<float>(children.size() - 1);
	spacingOffset = (std::max)(0.0f, spacingOffset);

	if (!expandedChildren.empty()) {
		const vec2 maxChildSize = {
			maxSize.x - state.padding.getSizeOffset().x,
			(std::max)(0.f, maxSize.y - state.padding.getSizeOffset().y - spacingOffset - totalHeight) / static_cast<float>(expandedChildren.size()),
		};
		for (auto &child: expandedChildren) {
			auto &childState = child->state;
			const auto childSize = child->layout(maxChildSize);
			if (childState.sizeMode.width.index() != 1 || std::get<1>(childState.sizeMode.width) != Size::Expand) {
				maxWidth = (std::max)(maxWidth, childSize.x);
			}
		}
	}

	minSize.x = maxWidth + state.padding.getSizeOffset().x;
	minSize.y = totalHeight + spacingOffset + state.padding.getSizeOffset().y;
}

void Column::Impl::arrangeChildren(vec2 &pos) {
	auto &children = getChildren();
	const auto width = getContentRect().width();
	auto cursor = pos + state.margin.getPositionOffset() + state.padding.getPositionOffset();
	const auto initialX = cursor.x;

	for (auto &child: children) {
		if (!child) continue;

		switch (alignment) {
			case Alignment::left:
				cursor.x = initialX;
				break;
			case Alignment::center:
				cursor.x = initialX + (width - child->getLayoutSize().x) / 2.0f;
				break;
			case Alignment::right:
				cursor.x = initialX + width - child->getLayoutSize().x;
				break;
		}

		child->arrange(cursor);
		cursor.y += child->getLayoutSize().y + spacing;
	}
}

void Column::Impl::drawChildren() {
	auto &children = getChildren();

	const Rect &clipRect = Renderer::getInstance().getCurrentClipRect().rect;

	for (auto &child: children) {
		if (!child) continue;
		if (child->getPos().y > clipRect.bottom) break;

		const float childHeight = child->getLayoutSize().y;
		if (child->getPos().y + childHeight < clipRect.top) continue;
		child->draw();
	}
}

float Column::Impl::getMinHeight(const vec2 &maxSize) {
	const auto &children = getChildren();
	if (!flags.visible) return 0.0f;
	switch (state.sizeMode.height.index()) {
		case 0: {
			return std::get<0>(state.sizeMode.height) + state.margin.getSizeOffset().y;
		}
		case 1: {
			const auto spacingOffset = spacing * static_cast<float>((std::max)(children.size(), 1ull) - 1);
			return state.margin.getSizeOffset().y + spacingOffset + state.padding.getSizeOffset().y + std::accumulate(children.begin(), children.end(), 0.0f, [&](float acc, const auto &child) {
					   return acc + child->getMinHeight(maxSize);
				   });
		}
		default: {
			std::unreachable();
			return 0.0f;
		}
	}
}
