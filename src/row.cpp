#include "row.hpp"
#include "ranges"
#include "window.hpp"
#include <algorithm>
#include <cmath>


using namespace squi;

Row::Impl::Impl(const Row &args)
	: Widget(args.widget, Widget::FlagsArgs::Default()),
	  alignment(args.alignment), spacing(args.spacing) {
	setChildren(args.children);
}

vec2 Row::Impl::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink) {
	auto children = getChildren() | std::views::filter([](const Child &child) {
						return *child->flags.visible;
					});
	const auto childCount = std::ranges::count_if(children, [](auto) {
		return true;
	});

	float totalWidth = 0.0f;
	float maxHeight = 0.0f;

	std::vector<Child> expandedChildren{};

	static bool ignoreHeight = false;
	if (!ignoreHeight && shouldShrink.height) {
		ignoreHeight = true;
		const auto size = layoutChildren(maxSize, minSize, shouldShrink).y;
		maxSize.y = std::clamp(size, minSize.y, maxSize.y);
		ignoreHeight = false;
		shouldShrink.height = false;
	}

	float spacingOffset = spacing * (static_cast<float>(childCount) - 1.f);
	spacingOffset = (std::max)(0.0f, spacingOffset);

	for (auto &child: children) {
		if (!child) continue;

		auto &childState = child->state;

		if (!shouldShrink.width && childState.width->index() == 1 && std::get<1>(*childState.width) == Size::Expand) {
			expandedChildren.emplace_back(child);
		} else {
			const auto childSize = child->layout(maxSize.withXOffset(-spacingOffset), {0, minSize.y}, shouldShrink);
			totalWidth += childSize.x;
			maxHeight = std::max(maxHeight, childSize.y);
		}
	}

	if (!expandedChildren.empty() && maxSize.x > totalWidth + spacingOffset) {
		const vec2 maxChildSize = {
			std::max(0.f, maxSize.x - spacingOffset - totalWidth) / static_cast<float>(expandedChildren.size()),
			maxSize.y,
		};
		for (auto &child: expandedChildren) {
			const auto childSize = child->layout(maxChildSize, {0, minSize.y}, shouldShrink);
			totalWidth += childSize.x;
			maxHeight = std::max(maxHeight, childSize.y);
		}
	}

	minSize.x = std::clamp(totalWidth + spacingOffset, minSize.x, maxSize.x);
	minSize.y = std::clamp(maxHeight, minSize.y, maxSize.y);

	return minSize;
}

void Row::Impl::arrangeChildren(vec2 &pos) {
	auto children = getChildren() | std::views::filter([](const Child &child) {
						return *child->flags.visible;
					});
	const auto height = getContentRect().height();
	auto cursor = pos + state.margin->getPositionOffset() + state.padding->getPositionOffset();
	const auto initialY = cursor.y;

	for (auto &child: children) {
		if (!child) continue;

		switch (alignment) {
			case Alignment::top:
				cursor.y = initialY;
				break;
			case Alignment::center:
				cursor.y = initialY + std::round((height - child->getLayoutSize().y) / 2.0f);
				break;
			case Alignment::bottom:
				cursor.y = initialY + height - child->getLayoutSize().y;
				break;
		}

		child->arrange(cursor);
		cursor.x += child->getLayoutSize().x + spacing;
	}
}

void Row::Impl::drawChildren() {
	auto children = getChildren() | std::views::filter([](const Child &child) {
						return *child->flags.visible;
					});

	const Rect clipRect = Window::of(this).engine.instance.scissorStack.back();

	for (auto &child: children) {
		if (!child) continue;
		if (child->getPos().x > clipRect.right) break;

		const float childWidth = child->getLayoutSize().x;
		if (child->getPos().x + childWidth < clipRect.left) continue;
		child->draw();
	}
}
