#include "row.hpp"

using namespace squi;

Row::Impl::Impl(const Row &args)
	: Widget(args.widget, Widget::Options{
							  .shouldUpdateChildren = false,
							  .shouldDrawChildren = false,
						  }),
	  alignment(args.alignment), spacing(args.spacing) {
	setChildren(args.children);
}

void Row::Impl::onUpdate() {
	auto &children = getChildren();

	float totalWidth = 0.0f;
	float maxHeight = 0.0f;

	std::vector<std::shared_ptr<Widget>> expandedChildren{};

	for (auto &child: children) {
		if (!child) continue;

		auto &childData = child->data();
		childData.parent = this;

		const auto childLayoutRect = child->getLayoutRect();

		if (childData.sizeBehavior.horizontal == SizeBehaviorType::FillParent) {
			expandedChildren.push_back(child);
		} else {
			child->update();
			totalWidth += childLayoutRect.width();
		}
		if (childData.sizeBehavior.vertical != SizeBehaviorType::FillParent) {
			maxHeight = std::max(maxHeight, childLayoutRect.height());
		}
	}

	auto &widgetData = data();

	float spacingOffset = spacing * (children.size() - 1);
	spacingOffset = std::max(0.0f, spacingOffset);

	const bool horizontalHint = widgetData.sizeHint.x != -1;
	const bool verticalHint = widgetData.sizeHint.y != -1;
	if (!horizontalHint && widgetData.sizeBehavior.horizontal == SizeBehaviorType::MatchChild && expandedChildren.empty()) {
		widgetData.size.x = totalWidth + spacingOffset + widgetData.padding.getSizeOffset().x;
	}
	if (!verticalHint && widgetData.sizeBehavior.vertical == SizeBehaviorType::MatchChild) {
		widgetData.size.y = maxHeight + widgetData.padding.getSizeOffset().y;
	}

	if (!expandedChildren.empty()) {
		const float expandedWidth = (widgetData.size.x - widgetData.padding.getSizeOffset().x - spacingOffset - totalWidth) / expandedChildren.size();
		for (auto &child: expandedChildren) {
			auto &childData = child->data();
			childData.sizeHint.x = expandedWidth;
			child->update();
		}
	}
}

void Row::Impl::matchChildSizeBehavior(bool horizontalHint, bool verticalHint) {
	// No-op
}

void Row::Impl::onDraw() {
	auto &widgetData = data();

	auto &children = getChildren();
	float cursor = 0.0f;
	const auto height = getContentRect().height();
	const auto childPos = widgetData.pos + widgetData.margin.getPositionOffset() + widgetData.padding.getPositionOffset();

	for (auto &child: children) {
		if (!child) continue;
		auto &childData = child->data();
		childData.pos.x = childPos.x + cursor;

		switch (alignment) {
			case Alignment::top:
				childData.pos.y = childPos.y;
				break;
			case Alignment::center:
				childData.pos.y = childPos.y + (height - child->getLayoutRect().height()) / 2.0f;
				break;
			case Alignment::bottom:
				childData.pos.y = childPos.y + height - child->getLayoutRect().height();
				break;
		}

        child->draw();
        cursor += child->getLayoutRect().width() + spacing;
	}
}