#include "child.hpp"
#include "widget.hpp"

using namespace squi;

void Child::initializeWidget() {
	widget->initialize();
}

Child::operator std::shared_ptr<Widget>() const {
	return widget;
}