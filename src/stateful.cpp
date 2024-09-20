#include "stateful.hpp"

#include "widget.hpp"

void squi::StatefulHelpers::reDraw(Widget *widget) {
	widget->reDraw();
}

void squi::StatefulHelpers::reLayout(Widget *widget) {
	widget->reLayout();
}

void squi::StatefulHelpers::reArrange(Widget *widget) {
	widget->reArrange();
}
