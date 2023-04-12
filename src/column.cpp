#include "column.hpp"
#include "renderer.hpp"

using namespace squi;

Column::Impl::Impl(const Column &args)
    : Widget(args.widget, Widget::Options{
        .shouldUpdateChildren = false,
        .shouldDrawChildren = false,}),
      alignment(args.alignment),
      spacing(args.spacing) {
    setChildren(args.children);
}

void Column::Impl::matchChildSizeBehavior(bool horizontalHint, bool verticalHint) {
    // No-op
}

void Column::Impl::onUpdate() {
    auto &children = getChildren();

    float totalHeight = 0.0f;
    float maxWidth = 0.0f;
    
    std::vector<std::shared_ptr<Widget>> expandedChildren{};

    for (auto &child: children) {
        if (!child) continue;

        auto &childData = child->data();
        childData.parent = this;

        const auto childLayoutRect = child->getLayoutRect();

        if (childData.sizeBehavior.vertical == SizeBehaviorType::FillParent) {
            expandedChildren.push_back(child);
        } else {
            child->update();
            totalHeight += childLayoutRect.height();
        }
        if (childData.sizeBehavior.horizontal != SizeBehaviorType::FillParent) {
            maxWidth = (std::max)(maxWidth, childLayoutRect.width());
        }
    }

    auto &widgetData = data();

    float spacingOffset = spacing * static_cast<float>(children.size() - 1);
    spacingOffset = (std::max)(0.0f, spacingOffset);

    const bool horizontalHint = widgetData.sizeHint.x != -1;
    const bool verticalHint = widgetData.sizeHint.y != -1;
    if (!horizontalHint && widgetData.sizeBehavior.horizontal == SizeBehaviorType::MatchChild) {
        widgetData.size.x = maxWidth + widgetData.padding.getSizeOffset().x;
    }
    if (!verticalHint && widgetData.sizeBehavior.vertical == SizeBehaviorType::MatchChild && expandedChildren.empty()) {
        widgetData.size.y = totalHeight + spacingOffset + widgetData.padding.getSizeOffset().y;
    }

    if (!expandedChildren.empty()) {
        const float expandedHeight = (widgetData.size.y - widgetData.padding.getSizeOffset().y - spacingOffset - totalHeight) / static_cast<float>(expandedChildren.size());
        for (auto &child: expandedChildren) {
            auto &childData = child->data();
            childData.sizeHint.y = expandedHeight;
            child->update();
        }
    }
}

void Column::Impl::onDraw() {
    auto &widgetData = data();

    auto &children = getChildren();
    float cursor = 0.0f;
    const auto width = getContentRect().width();
    const auto childPos = widgetData.pos + widgetData.margin.getPositionOffset() + widgetData.padding.getPositionOffset();

    const Rect &clipRect = Renderer::getInstance().getCurrentClipRect();

    for (auto &child: children) {
        if (!child) continue;
        auto &childData = child->data();
        childData.pos.y = childPos.y + cursor;
        if (childData.pos.y > clipRect.bottom) break;

        switch (alignment) {
            case Alignment::left:
                childData.pos.x = childPos.x;
                break;
            case Alignment::center:
                childData.pos.x = childPos.x + (width - child->getLayoutRect().width()) / 2.0f;
                break;
            case Alignment::right:
                childData.pos.x = childPos.x + width - child->getLayoutRect().width();
                break;
        }

        const float childHeight = child->getLayoutRect().height();
        cursor += childHeight + spacing;
        if (childData.pos.y + childHeight < clipRect.top) continue;
        child->draw();
    }
}