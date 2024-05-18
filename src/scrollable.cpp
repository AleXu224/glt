#include "scrollable.hpp"
#include "gestureDetector.hpp"
#include "window.hpp"
#include <algorithm>
#include <cmath>
#include <memory>

using namespace squi;

Scrollable::Impl::Impl(const Scrollable &args)
	: Widget(args.widget, Widget::FlagsArgs::Default()), spacing(args.spacing), horizontalOffsetFactor([&]() -> float {
		  switch (args.alignment) {
			  case Column::Alignment::left:
				  return 0.f;
			  case Column::Alignment::center:
				  return 0.5f;
			  case Column::Alignment::right:
				  return 1.f;
		  }
	  }()),
	  controller(args.controller) {
	setChildren(args.children);
	customState.add(args.controller->onScrollChange.observe([&self = *this](float newScroll) {
		if (newScroll != self.scroll) {
			self.scroll = newScroll;
			if (self.onScroll) self.onScroll(newScroll, self.controller->contentHeight, self.controller->viewHeight);
			self.reArrange();
		}
	}));
}

void Scrollable::Impl::onUpdate() {
	GestureDetector::g_activeArea.emplace_back(getRect());

	if (GestureDetector::canClick(*this) && GestureDetector::g_scrollDelta.y != 0.f) {
		scroll += GestureDetector::g_scrollDelta.y * -40.f;
	}

	if (controller->viewHeight > controller->contentHeight) {
		scroll = 0.f;
	} else {
		scroll = std::clamp(scroll, 0.f, controller->contentHeight - controller->viewHeight);
	}

	if (controller->scroll != scroll) {
		controller->scroll = scroll;
		if (onScroll) onScroll(scroll, controller->contentHeight, controller->viewHeight);
		reArrange();
	}
}

void squi::Scrollable::Impl::afterUpdate() {
	GestureDetector::g_activeArea.pop_back();
}

vec2 Scrollable::Impl::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool final) {
	auto &children = getChildren();

	float totalHeight = 0.f;
	float maxWidth = 0.f;

	for (auto &child: children) {
		if (!child) continue;

		const auto size = child->layout(maxSize.withY(std::numeric_limits<float>::max()), {minSize.x, 0}, shouldShrink, final);
		totalHeight += size.y;
		maxWidth = std::max(maxWidth, size.x);
	}
	float totalSpacing = spacing * (static_cast<float>(children.size()) - 1.f);
	totalSpacing = std::max(totalSpacing, 0.f);
	contentHeight = totalHeight + totalSpacing;

	return {maxWidth, contentHeight};
}

void Scrollable::Impl::postLayout(vec2 & /*size*/) {
	const float beforeScroll = scroll;

	const auto viewHeight = getContentRect().height();
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
	const auto childPos = pos + state.margin->getPositionOffset() + state.padding->getPositionOffset();
	float cursor = 0.f;
	const auto width = getSize().x;

	for (auto &child: children) {
		if (!child) continue;
		const auto horizontalOffset = (width - child->getLayoutSize().x) * horizontalOffsetFactor;
		child->arrange(
			childPos
				.withYOffset(-std::round(scroll - cursor))
				.withXOffset(horizontalOffset)
		);
		cursor += child->getLayoutSize().y + spacing;
	}
}

void Scrollable::Impl::drawChildren() {
	auto &children = getChildren();
	for (auto &child: children) {
		if (!child) continue;

		auto &instance = Window::of(this).engine.instance;
		instance.pushScissor(getRect());

		child->draw();

		instance.popScissor();
	}
}

squi::Scrollable::operator Child() const {
	return {std::make_shared<Impl>(*this)};
}
