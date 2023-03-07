#include "margin.hpp"

using namespace squi;

Margin Margin::operator+(const Margin &other) const {
	return Margin(top + other.top, right + other.right, bottom + other.bottom, left + other.left);
}

Margin Margin::operator-(const Margin &other) const {
	return Margin(top - other.top, right - other.right, bottom - other.bottom, left - other.left);
}

Margin Margin::operator*(const Margin &other) const {
	return Margin(top * other.top, right * other.right, bottom * other.bottom, left * other.left);
}

Margin Margin::operator/(const Margin &other) const {
	return Margin(top / other.top, right / other.right, bottom / other.bottom, left / other.left);
}

void Margin::operator+=(const Margin &other) {
	top += other.top;
	right += other.right;
	bottom += other.bottom;
	left += other.left;
}

void Margin::operator-=(const Margin &other) {
	top -= other.top;
	right -= other.right;
	bottom -= other.bottom;
	left -= other.left;
}

void Margin::operator*=(const Margin &other) {
	top *= other.top;
	right *= other.right;
	bottom *= other.bottom;
	left *= other.left;
}

void Margin::operator/=(const Margin &other) {
	top /= other.top;
	right /= other.right;
	bottom /= other.bottom;
	left /= other.left;
}

Margin Margin::withTop(const float &newTop) const {
	return Margin(newTop, right, bottom, left);
}

Margin Margin::withRight(const float &newRight) const {
	return Margin(top, newRight, bottom, left);
}

Margin Margin::withBottom(const float &newBottom) const {
	return Margin(top, right, newBottom, left);
}

Margin Margin::withLeft(const float &newLeft) const {
	return Margin(top, right, bottom, newLeft);
}

Margin Margin::withTopOffset(const float &offset) const {
	return Margin(top + offset, right, bottom, left);
}

Margin Margin::withRightOffset(const float &offset) const {
	return Margin(top, right + offset, bottom, left);
}

Margin Margin::withBottomOffset(const float &offset) const {
	return Margin(top, right, bottom + offset, left);
}

Margin Margin::withLeftOffset(const float &offset) const {
	return Margin(top, right, bottom, left + offset);
}

Margin Margin::withHorizontal(const float &newHorizontal) const {
	return Margin(top, newHorizontal, bottom, newHorizontal);
}

Margin Margin::withVertical(const float &newVertical) const {
	return Margin(newVertical, right, newVertical, left);
}

Margin Margin::withHorizontalOffset(const float &offset) const {
	return Margin(top, right + offset, bottom, left + offset);
}

Margin Margin::withVerticalOffset(const float &offset) const {
	return Margin(top + offset, right, bottom + offset, left);
}