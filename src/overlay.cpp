#include "overlay.hpp"
#include "widget.hpp"

using namespace squi;

Overlay::Impl::Impl(const Overlay &args)
	: Widget(args.widget, Widget::Flags::Default()) {
	addChild(args.child);
}

Overlay::operator Child() const {
	return {std::make_shared<Impl>(*this)};
}
