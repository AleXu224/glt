#include "stack.hpp"
#include "inputState.hpp"
#include "ranges"
#include <algorithm>
#include <vector>

using namespace squi;

Stack::Impl::Impl(const Stack &args)
	: Widget(args.widget, Widget::FlagsArgs::Default()) {
	setChildren(args.children);
}

void Stack::Impl::updateChildren() {
	auto &children = getChildren();
	auto &inputState = InputState::of(this);

	inputState.g_hitCheckRects.reserve(inputState.g_hitCheckRects.size() + children.size());
	uint32_t addedRects = 0;

	for (auto &child: std::views::reverse(children)) {
		if (!child->flags.visible) continue;
		child->update();
		const auto childHitcheckRect = child->getHitcheckRect();
		inputState.g_hitCheckRects.insert(inputState.g_hitCheckRects.end(), childHitcheckRect.begin(), childHitcheckRect.end());
		addedRects += childHitcheckRect.size();
	}

	for ([[maybe_unused]] uint32_t i = 0; i < addedRects; ++i) {
		inputState.g_hitCheckRects.pop_back();
	}
}

vec2 Stack::Impl::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool final) {
	auto &children = getChildren();


	if (shouldShrink.width || shouldShrink.height) {
		vec2 childrenMaxSize{};
		for (auto &child: children) {
			if (!child) continue;

			const auto size = child->layout(maxSize, minSize, shouldShrink, false);
			childrenMaxSize.x = std::max(size.x, childrenMaxSize.x);
			childrenMaxSize.y = std::max(size.y, childrenMaxSize.y);
		}

		if (shouldShrink.width)
			maxSize.x = std::clamp(childrenMaxSize.x, minSize.x, maxSize.x);
		if (shouldShrink.height)
			maxSize.y = std::clamp(childrenMaxSize.y, minSize.y, maxSize.y);
	}

	vec2 retSize{};
	for (auto &child: children) {
		if (!child) continue;

		// No shouldShrink provided since the widgets inside might want to expand
		// and we already calculated the max size they would be allowed to expand to
		const auto childSize = child->layout(maxSize, minSize, {}, final);

		retSize.x = std::max(retSize.x, childSize.x);
		retSize.y = std::max(retSize.y, childSize.y);
	}

	return retSize;
}

std::vector<Rect> Stack::Impl::getHitcheckRect() const {
	if (flags.isInteractive) {
		std::vector<Rect> ret{};
		for (const auto &child: getChildren()) {
			const auto childHitcheckRect = child->getHitcheckRect();
			ret.insert(ret.end(), childHitcheckRect.begin(), childHitcheckRect.end());
		}
		return ret;
	}
	return {};
}