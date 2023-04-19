#include "column.hpp"
#include "renderer.hpp"
#include "widget.hpp"
#include <tuple>

using namespace squi;

Column::Impl::Impl(const Column &args)
	: Widget(args.widget, Widget::Options{
							  .shouldDrawChildren = false,
							  .shouldHandleLayout = false,
                              .shouldArrangeChildren = false,
						  }),
	  alignment(args.alignment), spacing(args.spacing) {
	setChildren(args.children);
}

void Column::Impl::onLayout(vec2 &maxSize, vec2 &minSize) {
	auto &children = getChildren();

    float totalHeight = 0.0f;
    float maxWidth = 0.0f;
    
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
            totalHeight += childSize.y;
            if (childData.sizeMode.width.index() != 1 || std::get<1>(childData.sizeMode.width) != Size::Expand) {
                maxWidth = (std::max)(maxWidth, childSize.x);
            }
		}
    }

    auto &widgetData = data();

    float spacingOffset = spacing * static_cast<float>(children.size() - 1);
    spacingOffset = (std::max)(0.0f, spacingOffset);

    if (!expandedChildren.empty()) {
        const vec2 maxChildSize = {
            maxSize.x - widgetData.padding.getSizeOffset().x,
            (maxSize.y - widgetData.padding.getSizeOffset().y - spacingOffset - totalHeight) / static_cast<float>(expandedChildren.size()),
        };
        for (auto &child: expandedChildren) {
            auto &childData = child->data();
			const auto childSize = child->layout(maxChildSize);
			if (childData.sizeMode.width.index() != 1 || std::get<1>(childData.sizeMode.width) != Size::Expand) {
				maxWidth = (std::max)(maxWidth, childSize.x);
			}
		}
    }

    minSize.x = maxWidth + widgetData.padding.getSizeOffset().x;
    minSize.y = totalHeight + spacingOffset + widgetData.padding.getSizeOffset().y;
}

void Column::Impl::onArrange(vec2 &pos) {
    const auto &widgetData = data();
	auto &children = getChildren();
	const auto width = getContentRect().width();
	auto cursor = pos + widgetData.margin.getPositionOffset() + widgetData.padding.getPositionOffset();
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

void Column::Impl::onDraw() {
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