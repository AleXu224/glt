#include "widgetArgs.hpp"

using namespace squi;

Args squi::Args::withDefaultWidth(const Args::Width &newWidth) const {
	Args args = *this;
	args.width = this->width.value_or(newWidth);
	return args;
}

Args squi::Args::withDefaultHeight(const Args::Height &newHeight) const {
	Args args = *this;
	args.height = this->height.value_or(newHeight);
	return args;
}

Args squi::Args::withDefaultMargin(const Margin &newMargin) const {
	Args args = *this;
	args.margin = this->margin.value_or(newMargin);
	return args;
}
Args squi::Args::withDefaultPadding(const Padding &newPadding) const {
	Args args = *this;
	args.padding = this->padding.value_or(newPadding);
	return args;
}

Args squi::Args::withSizeConstraints(const SizeConstraints &newSizeConstraints) const {
	Args args = *this;
	args.sizeConstraints = newSizeConstraints;
	return args;
}
