#include "stack.hpp"
#include "gestureDetector.hpp"
#include <iterator>
#include <stdint.h>
#include <vector>
#include "ranges"

using namespace squi;

Stack::Impl::Impl(const Stack &args)
	: Widget(args.widget, Widget::Options{
							  .shouldUpdateChildren = false,
						  }) {
	setChildren(args.children);
}

void Stack::Impl::onUpdate() {
	auto &children = getChildren();

	GestureDetector::g_hitCheckRects.reserve(GestureDetector::g_hitCheckRects.size() + children.size());
	uint32_t addedRects = 0;

	for (auto &child: std::views::reverse(children)) {
		child->data().parent = this;
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