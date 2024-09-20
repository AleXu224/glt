#include "sizeConstraints.hpp"

using namespace squi;

SizeConstraints squi::SizeConstraints::withDefaultMinHeight(float value) const {
	auto copy = *this;
	copy.minHeight = this->minHeight.value_or(value);
	return copy;
}

SizeConstraints squi::SizeConstraints::withDefaultMaxWidth(float value) const {
	auto copy = *this;
	copy.maxWidth = this->maxWidth.value_or(value);
	return copy;
}

SizeConstraints squi::SizeConstraints::withDefaultMinWidth(float value) const {
	auto copy = *this;
	copy.minWidth = this->minWidth.value_or(value);
	return copy;
}

SizeConstraints squi::SizeConstraints::withDefaultMaxHeight(float value) const {
	auto copy = *this;
	copy.maxHeight = this->maxHeight.value_or(value);
	return copy;
}
