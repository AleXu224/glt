#include "borderRadius.hpp"

#include "glm/vec4.hpp"// IWYU pragma: keep

squi::BorderRadius::BorderRadius(float all) : topLeft(all), topRight(all), bottomRight(all), bottomLeft(all) {};
squi::BorderRadius::BorderRadius(float topLeft, float topRight, float bottomRight, float bottomLeft)
	: topLeft(topLeft),
	  topRight(topRight),
	  bottomRight(bottomRight),
	  bottomLeft(bottomLeft) {};

squi::BorderRadius squi::BorderRadius::TopLeft(float value) {
	return {value, 0.f, 0.f, 0.f};
}
squi::BorderRadius squi::BorderRadius::TopRight(float value) {
	return {0.f, value, 0.f, 0.f};
}
squi::BorderRadius squi::BorderRadius::BottomRight(float value) {
	return {0.f, 0.f, value, 0.f};
}
squi::BorderRadius squi::BorderRadius::BottomLeft(float value) {
	return {0.f, 0.f, 0.f, value};
}

[[nodiscard]] squi::BorderRadius squi::BorderRadius::withTopLeft(float value) {
	return {value, topRight, bottomRight, bottomLeft};
}
[[nodiscard]] squi::BorderRadius squi::BorderRadius::withTopRight(float value) {
	return {topLeft, value, bottomRight, bottomLeft};
}
[[nodiscard]] squi::BorderRadius squi::BorderRadius::withBottomRight(float value) {
	return {topLeft, topRight, value, bottomLeft};
}
[[nodiscard]] squi::BorderRadius squi::BorderRadius::withBottomLeft(float value) {
	return {topLeft, topRight, bottomRight, value};
}

squi::BorderRadius squi::BorderRadius::Top(float value) {
	return {value, value, 0.f, 0.f};
}
squi::BorderRadius squi::BorderRadius::Bottom(float value) {
	return {0.f, 0.f, value, value};
}
squi::BorderRadius squi::BorderRadius::Left(float value) {
	return {value, 0.f, 0.f, value};
}
squi::BorderRadius squi::BorderRadius::Right(float value) {
	return {0.f, value, value, 0.f};
}

[[nodiscard]] squi::BorderRadius squi::BorderRadius::withTop(float value) {
	return {value, value, bottomRight, bottomLeft};
}
[[nodiscard]] squi::BorderRadius squi::BorderRadius::withBottom(float value) {
	return {topLeft, topRight, value, value};
}
[[nodiscard]] squi::BorderRadius squi::BorderRadius::withLeft(float value) {
	return {value, topRight, bottomRight, value};
}
[[nodiscard]] squi::BorderRadius squi::BorderRadius::withRight(float value) {
	return {topLeft, value, value, bottomLeft};
}

squi::BorderRadius::operator glm::vec4() const {
	return {
		topLeft,
		topRight,
		bottomRight,
		bottomLeft,
	};
}