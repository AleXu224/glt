#include "margin.hpp"

using namespace squi;

Margin Margin::withTop(const float &newTop) const {
	return {newTop, right, bottom, left};
}

Margin Margin::withRight(const float &newRight) const {
	return {top, newRight, bottom, left};
}

Margin Margin::withBottom(const float &newBottom) const {
	return {top, right, newBottom, left};
}

Margin Margin::withLeft(const float &newLeft) const {
	return {top, right, bottom, newLeft};
}

Margin Margin::withTopOffset(const float &offset) const {
	return {top + offset, right, bottom, left};
}

Margin Margin::withRightOffset(const float &offset) const {
	return {top, right + offset, bottom, left};
}

Margin Margin::withBottomOffset(const float &offset) const {
	return {top, right, bottom + offset, left};
}

Margin Margin::withLeftOffset(const float &offset) const {
	return {top, right, bottom, left + offset};
}

Margin Margin::withHorizontal(const float &newHorizontal) const {
	return {top, newHorizontal, bottom, newHorizontal};
}

Margin Margin::withVertical(const float &newVertical) const {
	return {newVertical, right, newVertical, left};
}

Margin Margin::withHorizontalOffset(const float &offset) const {
	return {top, right + offset, bottom, left + offset};
}

Margin Margin::withVerticalOffset(const float &offset) const {
	return {top + offset, right, bottom + offset, left};
}

squi::Margin::operator glm::vec4() const {
	return {top, bottom, right, left};
}
