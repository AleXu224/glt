#include "row.hpp"
#include "renderer.hpp"
#include <numeric>

using namespace squi;

Row::Impl::Impl(const Row &args)
	: Widget(args.widget, Widget::Options{
							  .shouldDrawChildren = false,
							  .shouldHandleLayout = false,
							  .shouldArrangeChildren = false,
						  }),
	  alignment(args.alignment), spacing(args.spacing) {
	setChildren(args.children);
}

void Row::Impl::onLayout(vec2 &maxSize, vec2 &minSize) {
	auto &children = getChildren();

	float totalWidth = 0.0f;
	float maxHeight = 0.0f;

	std::vector<std::shared_ptr<Widget>> expandedChildren{};

	const vec2 maxChildSize = maxSize + data().padding.getSizeOffset();

	for (auto &child: children) {
		if (!child) continue;

		auto &childData = child->data();
		childData.parent = this;

		if (childData.sizeMode.height.index() == 1 && std::get<1>(childData.sizeMode.height) == Size::Expand) {
			expandedChildren.push_back(child);
		} else {
			const auto childSize = child->layout(maxChildSize);
			totalWidth += childSize.x;
			if (childData.sizeMode.height.index() != 1 || std::get<1>(childData.sizeMode.height) != Size::Expand) {
				maxHeight = (std::max)(maxHeight, childSize.y);
			}
		}
	}

	auto &widgetData = data();

	float spacingOffset = spacing * static_cast<float>(children.size() - 1);
	spacingOffset = (std::max)(0.0f, spacingOffset);

	if (!expandedChildren.empty()) {
		const vec2 maxChildSize = {
			(maxSize.x - widgetData.padding.getSizeOffset().x - spacingOffset - totalWidth) / static_cast<float>(expandedChildren.size()),
			maxSize.y - widgetData.padding.getSizeOffset().y,
		};
		for (auto &child: expandedChildren) {
			auto &childData = child->data();
			const auto childSize = child->layout(maxChildSize);
			if (childData.sizeMode.height.index() != 1 || std::get<1>(childData.sizeMode.height) != Size::Expand) {
				maxHeight = (std::max)(maxHeight, childSize.y);
			}
		}
	}

	minSize.x = totalWidth + spacingOffset + widgetData.padding.getSizeOffset().x;
	minSize.y = maxHeight + widgetData.padding.getSizeOffset().y;
}

void Row::Impl::onArrange(vec2 &pos) {
	auto &widgetData = data();
	auto &children = getChildren();
	const auto height = getContentRect().height();
	auto cursor = pos + widgetData.margin.getPositionOffset() + widgetData.padding.getPositionOffset();
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

void Row::Impl::onDraw() {
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

	const auto &widgetData = data();
	const auto &children = getChildren();
	switch (widgetData.sizeMode.width.index()) {
		case 0: {
			return std::get<0>(widgetData.sizeMode.width) + widgetData.margin.getSizeOffset().x;
		}
		case 1: {
			return widgetData.margin.getSizeOffset().x + widgetData.padding.getSizeOffset().x + std::accumulate(children.begin(), children.end(), 0.0f, [](float acc, const auto &child) {
					   return acc + child->getMinWidth();
				   });
		}
		default: {
			std::unreachable();
			return 0.0f;
		}
	}
}
