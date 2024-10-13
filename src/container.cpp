#include "container.hpp"

using namespace squi;

squi::Container::Impl::Impl(const Container &args) : Widget(args.widget, Widget::FlagsArgs::Default()), bounded(args.bounded) {
	addChild(args.child);
}

vec2 squi::Container::Impl::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool final) {
	vec2 contentSize{};

	auto newMaxSize = bounded ? maxSize : vec2::infinity();

	for (auto &child: getChildren()) {
		const auto size = child->layout(newMaxSize, minSize, shouldShrink, final);
		contentSize.x = std::max(size.x, contentSize.x);
		contentSize.y = std::max(size.y, contentSize.y);
	}

	return contentSize;
}
