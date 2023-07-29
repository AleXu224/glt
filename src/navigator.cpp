#include "navigator.hpp"
#include "widget.hpp"

using namespace squi;

void Navigator::Controller::push(const Child& child) {
    pushStack.push_back(child);
}

void Navigator::Controller::pop() {
    popCount++;
}

Navigator::Impl::Impl(const Navigator &args) : Widget(args.widget, Widget::Flags::Default()) {
}