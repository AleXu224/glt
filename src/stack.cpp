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
		child->update();
		const auto childHitcheckRect = child->getHitcheckRect();
		if (childHitcheckRect.has_value()) {
			GestureDetector::g_hitCheckRects.emplace_back(childHitcheckRect.value());
			++addedRects;
		}
	}

	for ([[maybe_unused]] uint32_t i = 0; i < addedRects; ++i) {
		GestureDetector::g_hitCheckRects.pop_back();
	}
}