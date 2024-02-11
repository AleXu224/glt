#include "navigator.hpp"
#include "widget.hpp"

using namespace squi;

void Navigator::Controller::push(Child child) {
	pushChild.swap(child);
}

void Navigator::Controller::pop() {
	shouldPop = true;
}

Navigator::Impl::Impl(const Navigator &args) : Widget(args.widget, Widget::FlagsArgs::Default()) {
	addChild(args.child);
}