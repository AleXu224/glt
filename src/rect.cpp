#include "rect.hpp"
#include "algorithm"

#include <array>

using namespace squi;

Rect::Rect(vec2 topLeft, vec2 bottomRight)
	: left(topLeft.x),
	  top(topLeft.y),
	  right(bottomRight.x),
	  bottom(bottomRight.y) {}

Rect Rect::overlap(const Rect &other) const {
	return {
		{(std::max) (left, other.left), (std::max) (top, other.top)},
		{(std::min) (right, other.right), (std::min) (bottom, other.bottom)},
	};
}
Rect Rect::transformed(const glm::mat4 &m) const {
	auto topLeft = this->getTopLeft();
	auto size = this->size();
	auto bottomRight = topLeft + size;
	auto topRight = topLeft.withXOffset(size.x);
	auto bottomLeft = topLeft.withYOffset(size.y);

	auto new_topLeft = m * glm::vec4(topLeft.x, topLeft.y, 0.f, 0.f);
	auto new_topRight = m * glm::vec4(topRight.x, topRight.y, 0.f, 0.f);
	auto new_bottomLeft = m * glm::vec4(bottomLeft.x, bottomLeft.y, 0.f, 0.f);
	auto new_bottomRight = m * glm::vec4(bottomRight.x, bottomRight.y, 0.f, 0.f);

	auto xVals = std::array<float, 4>{new_topLeft.x, new_topRight.x, new_bottomLeft.x, new_bottomRight.x};
	auto yVals = std::array<float, 4>{new_topLeft.y, new_topRight.y, new_bottomLeft.y, new_bottomRight.y};

	auto minX = std::ranges::min_element(xVals);
	auto maxX = std::ranges::max_element(xVals);
	auto minY = std::ranges::min_element(yVals);
	auto maxY = std::ranges::max_element(yVals);

	return {
		vec2{*minX, *minY},
		vec2{*maxX, *maxY}
	};
}
[[nodiscard]] squi::Rect squi::Rect::dragInside(Rect other) const {
	if (other.left < left) {
		other.offset(vec2{left - other.left, 0.f});
	} else if (other.right > right) {
		other.offset(vec2{right - other.right, 0.f});
	}
	if (other.top < top) {
		other.offset(vec2{0.f, top - other.top});
	} else if (other.bottom > bottom) {
		other.offset(vec2{0.f, bottom - other.bottom});
	}
	return other;
}
[[nodiscard]] squi::Rect squi::Rect::rounded() const {
	auto pos = getTopLeft().rounded();
	auto size = this->size().rounded();
	return Rect::fromPosSize(pos, size);
}
