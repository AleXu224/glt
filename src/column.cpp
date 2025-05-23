#include "column.hpp"
#include "ranges"
#include "window.hpp"
#include <algorithm>
#include <cmath>


using namespace squi;

Column::Impl::Impl(const Column &args)
	: Widget(args.widget, Widget::FlagsArgs::Default()),
	  alignment(args.alignment), spacing(args.spacing) {
	setChildren(args.children);
}

vec2 Column::Impl::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool final) {
	auto children = getChildren() | std::views::filter([](const Child &child) {
						return *child->flags.visible;
					});
	const auto childCount = std::ranges::count_if(children, [](const auto &) {
		return true;
	});

	float totalHeight = 0.0f;
	float maxWidth = 0.0f;

	std::vector<Child> expandedChildren{};
	std::vector<Child> wrapedChildren{};

	static bool ignoreWidth = false;
	if (!ignoreWidth && shouldShrink.width) {
		ignoreWidth = true;
		const auto size = layoutChildren(maxSize, minSize, shouldShrink, false).x;
		maxSize.x = std::clamp(size, minSize.x, maxSize.x);
		ignoreWidth = false;
		shouldShrink.width = false;
	}

	float spacingOffset = spacing * (static_cast<float>(childCount) - 1.f);
	spacingOffset = (std::max) (0.0f, spacingOffset);

	for (auto &child: children) {
		if (!child) continue;

		auto &childState = child->state;

		if (!shouldShrink.height && childState.height->index() == 1 && std::get<1>(*childState.height) == Size::Expand) {
			expandedChildren.emplace_back(child);
		} else if (childState.height->index() == 1 && std::get<1>(*childState.height) == Size::Wrap) {
			wrapedChildren.emplace_back(child);
		} else {
			const auto childSize = child->layout(maxSize.withYOffset(-spacingOffset), {minSize.x, 0}, shouldShrink, final);
			totalHeight += childSize.y;
			maxWidth = std::max(maxWidth, childSize.x);
		}
	}

	if (!wrapedChildren.empty() && maxSize.y > totalHeight + spacingOffset) {
		const vec2 maxChildSize = {
			maxSize.x,
			std::max(0.f, maxSize.y - spacingOffset - totalHeight) / static_cast<float>(wrapedChildren.size()),
		};
		for (auto &child: wrapedChildren) {
			const auto childSize = child->layout(maxChildSize, {minSize.x, 0}, shouldShrink, final);
			totalHeight += childSize.y;
			maxWidth = std::max(maxWidth, childSize.x);
		}
	}

	if (!expandedChildren.empty() && maxSize.y > totalHeight + spacingOffset) {
		const vec2 maxChildSize = {
			maxSize.x,
			std::max(0.f, maxSize.y - spacingOffset - totalHeight) / static_cast<float>(expandedChildren.size()),
		};
		for (auto &child: expandedChildren) {
			const auto childSize = child->layout(maxChildSize, {minSize.x, 0}, shouldShrink, final);
			totalHeight += childSize.y;
			maxWidth = std::max(maxWidth, childSize.x);
		}
	}

	minSize.y = std::clamp(totalHeight + spacingOffset, minSize.y, maxSize.y);
	minSize.x = std::clamp(maxWidth, minSize.x, maxSize.x);

	return minSize;
}

void Column::Impl::arrangeChildren(vec2 &pos) {
	auto children = getChildren() | std::views::filter([](const Child &child) {
						return *child->flags.visible;
					});
	const auto width = getContentRect().width();
	auto cursor = pos + state.margin->getPositionOffset() + state.padding->getPositionOffset();
	const auto initialX = cursor.x;

	for (auto &child: children) {
		if (!child) continue;

		switch (alignment) {
			case Alignment::left:
				cursor.x = initialX;
				break;
			case Alignment::center:
				cursor.x = initialX + std::round((width - child->getLayoutSize().x) / 2.0f);
				break;
			case Alignment::right:
				cursor.x = initialX + width - child->getLayoutSize().x;
				break;
		}

		child->arrange(cursor);
		cursor.y += child->getLayoutSize().y + spacing;
	}
}

void Column::Impl::drawChildren() {
	auto children = getChildren() | std::views::filter([](const Child &child) {
						return *child->flags.visible;
					});

	const Rect clipRect = Window::of(this).engine.instance.scissorStack.back();

	for (auto &child: children) {
		if (!child) continue;
		if (child->getPos().y > clipRect.bottom) break;

		const float childHeight = child->getLayoutSize().y;
		if (child->getPos().y + childHeight < clipRect.top) continue;
		child->draw();
	}
}
