#include "stack.hpp"
#include "gestureDetector.hpp"
#include "ranges"
#include <iterator>
#include <vector>

using namespace squi;

Stack::Impl::Impl(const Stack &args)
	: Widget(args.widget, Widget::Flags::Default()) {
	setChildren(args.children);
}

void Stack::Impl::updateChildren() {
	auto &children = getChildren();

	GestureDetector::g_hitCheckRects.reserve(GestureDetector::g_hitCheckRects.size() + children.size());
	uint32_t addedRects = 0;

	for (auto &child: std::views::reverse(children)) {
		child->state.parent = this;
		child->state.root = state.root;
		child->update();
		const auto childHitcheckRect = child->getHitcheckRect();
		GestureDetector::g_hitCheckRects.insert(GestureDetector::g_hitCheckRects.end(), childHitcheckRect.begin(), childHitcheckRect.end());
		addedRects += childHitcheckRect.size();
	}

	for ([[maybe_unused]] uint32_t i = 0; i < addedRects; ++i) {
		GestureDetector::g_hitCheckRects.pop_back();
	}
}

std::vector<Rect> Stack::Impl::getHitcheckRect() const {
	if (flags.isInteractive) {
		std::vector<Rect> ret{};
		for (auto &child: getChildren()) {
			const auto childHitcheckRect = child->getHitcheckRect();
			ret.insert(ret.end(), childHitcheckRect.begin(), childHitcheckRect.end());
		}
		return ret;
	} else {
		return {};
	}
}