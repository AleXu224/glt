#include "gestureDetector.hpp"
#include <any>
#define NOMINMAX
#include "renderer.hpp"
#include "scrollable.hpp"


using namespace squi;

Scrollable::Impl::Impl(const Scrollable &args)
	: Widget(args.widget, Widget::Flags::Default()) {
	addChild(Column{
		.widget{
			.width = Size::Expand,
			.height = Size::Shrink,
			.sizeConstraints{
				.maxHeight = std::numeric_limits<float>::max(),
			},
			.afterUpdate = [&, onScroll = args.onScroll, setScroll = args.setScroll](Widget &widget) {
				const auto viewHeight = getContentRect().height();
				const auto contentHeight = widget.getLayoutRect().height();
				const auto maxScroll = contentHeight - viewHeight;

				if (setScroll) {
					const float newScroll = setScroll();
					if (!scrolled && newScroll != scroll) {
						scroll = newScroll;
						if (onScroll) onScroll(scroll, contentHeight, viewHeight);
						return;
					}
				};

				if (viewHeight > contentHeight) scroll = 0;
				scroll = (std::min)(scroll, maxScroll);
				scroll = (std::max)(0.0f, scroll);

				if (onScroll) onScroll(scroll, contentHeight, viewHeight);
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

	if (gd.hovered) {
		scroll += GestureDetector::g_scrollDelta.y * -40.0f;
		if (GestureDetector::g_scrollDelta.y != 0) scrolled = true;
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
