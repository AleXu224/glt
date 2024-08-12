#include "softWrap.hpp"

using namespace squi;

vec2 squi::SoftWrap::Impl::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink /*shouldShrink*/, bool final) {
	auto &children = getChildren();
	vec2 maxChildSize{};
	for (auto &child: children) {
		auto size = child->layout(maxSize, minSize, {false, false}, final);
		maxChildSize.x = std::max(maxChildSize.x, size.x);
		maxChildSize.y = std::max(maxChildSize.y, size.y);
	}

	return maxChildSize;
}

squi::SoftWrap::Impl::Impl(const SoftWrap &args) : Widget(args.widget, Widget::FlagsArgs::Default()) {
	addChild(args.child);
}
