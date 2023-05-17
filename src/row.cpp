#include "row.hpp"
#include "renderer.hpp"
#include <numeric>

using namespace squi;

Row::Impl::Impl(const Row &args)
	: Widget(args.widget, Widget::Flags::Default()),
	  alignment(args.alignment), spacing(args.spacing) {
	setChildren(args.children);
}

void Row::Impl::layoutChildren(vec2 &maxSize, vec2 &minSize) {
	auto &children = getChildren();

	float totalWidth = 0.0f;
	float maxHeight = 0.0f;

	std::vector<std::shared_ptr<Widget>> expandedChildren{};

	const vec2 maxChildSize = maxSize + state.padding.getSizeOffset();

	for (auto &child: children) {
		if (!child) continue;

		auto &childState = child->state;
		childState.parent = this;

		if (childState.sizeMode.width.index() == 1 && std::get<1>(childState.sizeMode.width) == Size::Expand) {
			expandedChildren.push_back(child);
		} else {
			const auto childSize = child->layout(maxChildSize);
			totalWidth += childSize.x;
			if (childState.sizeMode.height.index() != 1 || std::get<1>(childState.sizeMode.height) != Size::Expand) {
				maxHeight = (std::max)(maxHeight, childSize.y);
			}
		}
	}

	float spacingOffset = spacing * static_cast<float>(children.size() - 1);
	spacingOffset = (std::max)(0.0f, spacingOffset);

	if (!expandedChildren.empty()) {
		const vec2 maxChildSize = {
			(maxSize.x - state.padding.getSizeOffset().x - spacingOffset - totalWidth) / static_cast<float>(expandedChildren.size()),
			maxSize.y - state.padding.getSizeOffset().y,
		};
		for (auto &child: expandedChildren) {
			auto &childState = child->state;
			const auto childSize = child->layout(maxChildSize);
			if (childState.sizeMode.height.index() != 1 || std::get<1>(childState.sizeMode.height) != Size::Expand) {
				maxHeight = (std::max)(maxHeight, childSize.y);
			}
		}
	}

	minSize.x = totalWidth + spacingOffset + state.padding.getSizeOffset().x;
	minSize.y = maxHeight + state.padding.getSizeOffset().y;
}

void Row::Impl::arrangeChildren(vec2 &pos) {
	auto &children = getChildren();
	const auto height = getContentRect().height();
	auto cursor = pos + state.margin.getPositionOffset() + state.padding.getPositionOffset();
	const auto initialY = cursor.y;

	for (auto &child: children) {
		if (!child) continue;

		switch (alignment) {
			case Alignment::top:
				cursor.y = initialY;
				break;
			case Alignment::center:
				cursor.y = initialY + (height - child->getLayoutSize().y) / 2.0f;
				break;
			case Alignment::bottom:
				cursor.y = initialY + height - child->getLayoutSize().y;
				break;
		}

		child->arrange(cursor);
		cursor.x += child->getLayoutSize().x + spacing;
	}
}

void Row::Impl::drawChildren() {
	auto &children = getChildren();
	const Rect &clipRect = Renderer::getInstance().getCurrentClipRect().rect;

	for (auto &child: children) {
		if (!child) continue;
		if (child->getPos().x > clipRect.right) break;

		const float childWidth = child->getLayoutSize().x;
		if (child->getPos().x + childWidth < clipRect.left) continue;
		child->draw();
	}
}

float squi::Row::Impl::getMinWidth() {
	const auto &children = getChildren();
	if (!flags.visible) return 0.0f;
	switch (state.sizeMode.width.index()) {
		case 0: {
			return std::get<0>(state.sizeMode.width) + state.margin.getSizeOffset().x;
		}
		case 1: {
			return state.margin.getSizeOffset().x + state.padding.getSizeOffset().x + std::accumulate(children.begin(), children.end(), 0.0f, [](float acc, const auto &child) {
					   return acc + child->getMinWidth();
				   });
		}
		default: {
			std::unreachable();
			return 0.0f;
		}
	}
}
