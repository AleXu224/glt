#include "scrollable.hpp"
#include "renderer.hpp"

using namespace squi;

Scrollable::Impl::Impl(const Scrollable &args)
    : Widget(args.widget, Widget::Options{
        .shouldDrawChildren = false,
    }) {
	addChild(Column{
		.widget{
			.sizeBehavior{
				.horizontal = SizeBehaviorType::FillParent,
				.vertical = SizeBehaviorType::MatchChild,
			},
			.beforeUpdate = [&](Widget &widget) {
                const auto scrollableHeight = getContentRect().height();
                const auto childHeight = widget.getLayoutRect().height();
                const auto maxScroll = childHeight - scrollableHeight;

                if (scrollableHeight > childHeight) scroll = 0;
                scroll = (std::min)(scroll, maxScroll);
				scroll = (std::max)(0.0f, scroll);
			},
		},
        .alignment = args.alignment,
		.spacing = args.spacing,
		.children = args.children,
	});
}

void Scrollable::Impl::onUpdate() {
    auto &widgetData = this->data();
    auto &gd = widgetData.gestureDetector;

    if (gd.hovered) {
        scroll += gd.g_scrollDelta.y * -40.0f;
    }
}

void Scrollable::Impl::onDraw() {
    auto &children = getChildren();
    if (children.empty()) return;
    auto &child = children[0];
    if (!child) return;

    auto &widgetData = this->data();

    auto &renderer = Renderer::getInstance();
    renderer.addClipRect(getRect());

    const auto pos = widgetData.pos + widgetData.margin.getPositionOffset() + widgetData.padding.getPositionOffset();
    auto &childData = child->data();
    childData.pos = pos.withYOffset(-scroll);
    child->draw();

    renderer.popClipRect();
}
