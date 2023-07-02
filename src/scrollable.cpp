#include "gestureDetector.hpp"
#include <any>
#include <memory>
#define NOMINMAX
#include "renderer.hpp"
#include "scrollable.hpp"


using namespace squi;

Scrollable::Impl::Impl(const Scrollable &args)
	: Widget(args.widget, Widget::Flags::Default()), controller(args.controller) {
	addChild(Column{
		.widget{
			.width = Size::Expand,
			.height = Size::Shrink,
			.sizeConstraints{
				.maxHeight = std::numeric_limits<float>::max(),
			},
			.afterUpdate = [this, onScroll = args.onScroll](Widget &widget) {
				const float beforeScroll = scroll;
				
				const auto viewHeight = getContentRect().height();
				const auto contentHeight = widget.getLayoutRect().height();
				const auto maxScroll = contentHeight - viewHeight;

				if (controller->scroll != scroll) {
					scroll = controller->scroll;
					if (onScroll) onScroll(scroll, contentHeight, viewHeight);
				}

				controller->viewHeight = viewHeight;
				controller->contentHeight = contentHeight;

				if (viewHeight > contentHeight) scroll = 0;
				scroll = (std::min)(scroll, maxScroll);
				scroll = (std::max)(0.0f, scroll);

				controller->scroll = scroll;

				if (beforeScroll != scroll && onScroll) 
					onScroll(scroll, contentHeight, viewHeight);
			},
		},
		.alignment = args.alignment,
		.spacing = args.spacing,
		.children = args.children,
	});
}

void Scrollable::Impl::onUpdate() {
	scrolled = false;
	auto &gd = std::any_cast<GestureDetector::Storage&>(state.properties.at("gestureDetector"));

	if (gd.hovered && GestureDetector::g_scrollDelta.y != 0) {
		scroll += GestureDetector::g_scrollDelta.y * -40.0f;
		if (GestureDetector::g_scrollDelta.y != 0) scrolled = true;
		controller->scroll = scroll;
	}

	GestureDetector::g_activeArea.emplace_back(getRect());
}

void squi::Scrollable::Impl::afterUpdate() {
	GestureDetector::g_activeArea.pop_back();
}

void Scrollable::Impl::arrangeChildren(vec2 &pos) {
	auto &children = getChildren();
	if (children.empty()) return;
	auto &child = children[0];
	if (!child) return;

	const auto childPos = pos + state.margin.getPositionOffset() + state.padding.getPositionOffset();
	child->arrange(childPos.withYOffset(-scroll));
}

void Scrollable::Impl::drawChildren() {
	auto &children = getChildren();
	if (children.empty()) return;
	auto &child = children[0];
	if (!child) return;

	auto &renderer = Renderer::getInstance();
	renderer.addClipRect(getRect());

	child->draw();

	renderer.popClipRect();
}

squi::Scrollable::operator Child() const {
	return GestureDetector{.child = {std::make_shared<Impl>(*this)}};
}
