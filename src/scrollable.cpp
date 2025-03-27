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
			  case Alignment::begin:
				  return 0.f;
			  case Alignment::center:
				  return 0.5f;
			  case Alignment::end:
				  return 1.f;
		  }
		  std::unreachable();
	  }()),
	  direction(args.direction),
	  controller(args.controller) {
	setChildren(args.children);
	customState.add(args.controller->onScrollChange.observe([&self = *this](float newScroll) {
		if (newScroll != self.scroll) {
			self.scroll = newScroll;
			if (self.onScroll) self.onScroll(newScroll, self.controller->contentMainAxis, self.controller->viewMainAxis);
			self.reArrange();
		}
	}));
}

void Scrollable::Impl::onUpdate() {
	auto &inputState = InputState::of(this);
	inputState.g_activeArea.emplace_back(getRect());

	auto &delta = [&]() -> float & {
		switch (direction) {
			case Direction::vertical:
				return inputState.g_scrollDelta.y;
				break;
			case Direction::horizontal:
				return inputState.g_scrollDelta.x;
				break;
		}
	}();
	if (GestureDetector::canClick(*this) && delta != 0.f) {
		scroll += delta * -40.f;
	}

	if (controller->viewMainAxis > controller->contentMainAxis) {
		scroll = 0.f;
	} else {
		scroll = std::clamp(scroll, 0.f, controller->contentMainAxis - controller->viewMainAxis);
	}

	if (controller->scroll != scroll) {
		controller->scroll = scroll;
		if (onScroll) onScroll(scroll, controller->contentMainAxis, controller->viewMainAxis);
		reArrange();
	}
}

void squi::Scrollable::Impl::afterUpdate() {
	InputState::of(this).g_activeArea.pop_back();
}

vec2 Scrollable::Impl::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool final) {
	auto &children = getChildren();

	auto newMinSize = minSize;
	auto newMaxSize = maxSize;
	float totalMainAxis = 0.f;
	float maxCrossAxis = 0.f;

	switch (direction) {
		case Direction::vertical:
			shouldShrink.height = true;
			newMaxSize = newMaxSize.withY(std::numeric_limits<float>::max());
			minSize = minSize.withY(0.f);
			break;
		case Direction::horizontal:
			shouldShrink.width = true;
			newMaxSize = newMaxSize.withX(std::numeric_limits<float>::max());
			minSize = minSize.withX(0.f);
			break;
	}

	for (auto &child: children) {
		if (!child) continue;

		const auto size = child->layout(newMaxSize, newMinSize, shouldShrink, final);
		switch (direction) {
			case Direction::vertical:
				totalMainAxis += size.y;
				maxCrossAxis = std::max(maxCrossAxis, size.x);
				break;
			case Direction::horizontal:
				totalMainAxis += size.x;
				maxCrossAxis = std::max(maxCrossAxis, size.y);
				break;
		}
	}
	float totalSpacing = spacing * (static_cast<float>(children.size()) - 1.f);
	totalSpacing = std::max(totalSpacing, 0.f);
	contentMainAxis = totalMainAxis + totalSpacing;

	switch (direction) {
		case Direction::vertical:
			return {maxCrossAxis, std::min(contentMainAxis, maxSize.y)};
		case Direction::horizontal:
			return {std::min(contentMainAxis, maxSize.x), maxCrossAxis};
	}
}

void Scrollable::Impl::postLayout(vec2 & /*size*/) {
	const float beforeScroll = scroll;

	auto viewMainAxis = 0.f;

	switch (direction) {
		case Direction::vertical:
			viewMainAxis = getContentRect().height();
			break;
		case Direction::horizontal:
			viewMainAxis = getContentRect().width();
			break;
	}

	const auto maxScroll = contentMainAxis - viewMainAxis;

	controller->viewMainAxis = viewMainAxis;
	controller->contentMainAxis = contentMainAxis;

	if (viewMainAxis > contentMainAxis) {
		scroll = 0;
	} else {
		scroll = std::clamp(scroll, 0.0f, maxScroll);
		controller->scroll = scroll;
	}

	if (beforeScroll != scroll) {
		if (onScroll) onScroll(scroll, contentMainAxis, viewMainAxis);
	}
}

void Scrollable::Impl::arrangeChildren(vec2 &pos) {
	auto &children = getChildren();
	const auto childPos = pos + state.margin->getPositionOffset() + state.padding->getPositionOffset();
	float cursor = 0.f;
	const auto crossAxisWidth = [&]() {
		switch (direction) {
			case Direction::vertical:
				return getContentSize().x;
			case Direction::horizontal:
				return getContentSize().y;
		}
	}();

	for (auto &child: children) {
		if (!child) continue;
		switch (direction) {
			case Direction::vertical: {
				auto offset = (crossAxisWidth - child->getLayoutSize().x) * horizontalOffsetFactor;
				child->arrange(
					childPos
						.withYOffset(-std::round(scroll - cursor))
						.withXOffset(offset)
				);
				cursor += child->getLayoutSize().y + spacing;
				break;
			}
			case Direction::horizontal: {
				auto offset = (crossAxisWidth - child->getLayoutSize().y) * horizontalOffsetFactor;
				child->arrange(
					childPos
						.withXOffset(-std::round(scroll - cursor))
						.withYOffset(offset)
				);
				cursor += child->getLayoutSize().x + spacing;
				break;
			}
		}
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
