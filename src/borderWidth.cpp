#include "borderWidth.hpp"

#include "glm/vec4.hpp"// IWYU pragma: keep

squi::BorderWidth::BorderWidth(float all) : top(all), right(all), bottom(all), left(all) {}
squi::BorderWidth::BorderWidth(float top, float right, float bottom, float left) : top(top), right(right), bottom(bottom), left(left) {}

squi::BorderWidth squi::BorderWidth::Top(float width) {
	return {width, 0.f, 0.f, 0.f};
};
squi::BorderWidth squi::BorderWidth::Right(float width) {
	return {0.f, width, 0.f, 0.f};
};
squi::BorderWidth squi::BorderWidth::Bottom(float width) {
	return {0.f, 0.f, width, 0.f};
};
squi::BorderWidth squi::BorderWidth::Left(float width) {
	return {0.f, 0.f, 0.f, width};
};

squi::BorderWidth squi::BorderWidth::withTop(float width) const {
	return {width, right, bottom, left};
}
squi::BorderWidth squi::BorderWidth::withRight(float width) const {
	return {top, width, bottom, left};
}
squi::BorderWidth squi::BorderWidth::withBottom(float width) const {
	return {top, right, width, left};
}
squi::BorderWidth squi::BorderWidth::withLeft(float width) const {
	return {top, right, bottom, width};
}

squi::BorderWidth::operator glm::vec4() const {
	return {top, right, bottom, left};
}
bool squi::BorderWidth::operator==(const BorderWidth &other) const {
	return left == other.left
		&& right == other.right
		&& top == other.top
		&& bottom == other.bottom;
}
