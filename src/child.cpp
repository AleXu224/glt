#include "child.hpp"
#include "widget.hpp"

using namespace squi;

void Child::initializeWidget() {
	if (widget)	{
		widget->initialize();
		Widget::Store::widgets.insert({widget->id, widget});
	}
}

Child::operator std::shared_ptr<Widget>() const {
	return widget;
}

Children::operator std::vector<std::shared_ptr<Widget>>() const {
	std::vector<std::shared_ptr<Widget>> widgets{};
	widgets.reserve(children.size());
	for (auto &child: children) {
		widgets.push_back(child);
	}
	return widgets;
}