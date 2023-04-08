#include "stack.hpp"

using namespace squi;

Stack::Impl::Impl(const Stack &args)
	: Widget(args.widget, Widget::Options::Default()) {
	setChildren(args.children);
}