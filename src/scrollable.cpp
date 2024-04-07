#include "scrollable.hpp"
#include "gestureDetector.hpp"
#include "window.hpp"
#include <algorithm>
#include <cmath>
#include <memory>

using namespace squi;

Scrollable::Impl::Impl(const Scrollable &args)
	: Widget(args.widget, Widget::FlagsArgs::Default()), controller(args.controller) {
	GestureDetector{
		.onUpdate = [this](GestureDetector::Event event) {
			scrolled = false;
			if (controller->viewHeight > controller->contentHeight) return;

			if (event.state.hovered && GestureDetector::g_scrollDelta.y != 0) {
				scroll += GestureDetector::g_scrollDelta.y * -40.0f;
				if (GestureDetector::g_scrollDelta.y != 0) scrolled = true;
				controller->scroll = scroll;
				reArrange();
			}
		},
	}
		.mount(*this);
	addChild(Column{
		.widget{
			.width = Size::Expand,
			.height = Size::Shrink,
			.sizeConstraints{
				.maxHeight = std::numeric_limits<float>::max(),
			},
			.afterUpdate = [this](Widget & /*widget*/) {
				const float beforeScroll = scroll;

				const auto contentHeight = controller->contentHeight;
				const auto viewHeight = controller->viewHeight;

				if (controller->scroll != scroll) {
					scroll = controller->scroll;
					if (onScroll) onScroll(scroll, contentHeight, viewHeight);
					this->reArrange();
				}

				if (viewHeight > contentHeight) {
					scroll = 0;
				} else {
					scroll = std::clamp(scroll, 0.0f, contentHeight - viewHeight);
				}
				controller->scroll = scroll;

				if (beforeScroll != scroll) {
					if (onScroll) onScroll(scroll, contentHeight, viewHeight);
					this->reArrange();
				}
			},
			.afterLayout = [this](Widget &widget, auto) {
				const auto contentHeight = widget.getLayoutRect().height();
				controller->contentHeight = contentHeight;
			},
		},
		.alignment = args.alignment,
		.spacing = args.spacing,
		.children = args.children,
	});
}

void Scrollable::Impl::onUpdate() {
	GestureDetector::g_activeArea.emplace_back(getRect());
}

void squi::Scrollable::Impl::afterUpdate() {
	GestureDetector::g_activeArea.pop_back();
}

vec2 Scrollable::Impl::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink) {
	auto &children = getChildren();

	for (auto &child: children) {
		if (!child) continue;

		const auto size = child->layout(maxSize.withY(std::numeric_limits<float>::max()), {minSize.x, 0}, shouldShrink);
		minSize.x = std::clamp(size.x, minSize.x, maxSize.x);
		minSize.y = std::clamp(size.y, minSize.y, maxSize.y);
	}

	return minSize;
}

void Scrollable::Impl::postLayout(vec2 & /*size*/) {
	const float beforeScroll = scroll;

	const auto viewHeight = getContentRect().height();
	const auto contentHeight = controller->contentHeight;
	const auto maxScroll = contentHeight - viewHeight;

	controller->viewHeight = viewHeight;
	controller->contentHeight = contentHeight;

	if (viewHeight > contentHeight) {
		scroll = 0;
	} else {
		scroll = std::clamp(scroll, 0.0f, maxScroll);
		controller->scroll = scroll;
	}

	if (beforeScroll != scroll) {
		if (onScroll) onScroll(scroll, contentHeight, viewHeight);
	}
}

void Scrollable::Impl::arrangeChildren(vec2 &pos) {
	auto &children = getChildren();
	if (children.empty()) return;
	auto &child = children.front();
	if (!child) return;

	const auto childPos = pos + state.margin->getPositionOffset() + state.padding->getPositionOffset();
	child->arrange(childPos.withYOffset(-std::round(scroll)));
}

void Scrollable::Impl::drawChildren() {
	auto &children = getChildren();
	if (children.empty()) return;
	auto &child = children.front();
	if (!child) return;

	auto &instance = Window::of(this).engine.instance;
	instance.pushScissor(getRect());

	child->draw();

	instance.popScissor();
}

squi::Scrollable::operator Child() const {
	return {std::make_shared<Impl>(*this)};
}
