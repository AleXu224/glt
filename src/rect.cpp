#include "rect.hpp"
#include "algorithm"

using namespace squi;

Rect::Rect(vec2 topLeft, vec2 bottomRight)
	: left(topLeft.x),
	  top(topLeft.y),
	  right(bottomRight.x),
	  bottom(bottomRight.y) {}

// Rect Rect::fromPosSize(vec2 position, vec2 size) {
// 	return {position, position + size};
// }

// float Rect::width() const {
// 	return right - left;
// }

// float Rect::height() const {
// 	return bottom - top;
// }

// vec2 Rect::size() const {
// 	return {width(), height()};
// }

// bool Rect::contains(const vec2 &position) const {
// 	return position.x >= left && position.x < right && position.y >= top && position.y < bottom;
// }

Rect Rect::overlap(const Rect &other) const {
	return {
		{(std::max)(left, other.left), (std::max)(top, other.top)},
		{(std::min)(right, other.right), (std::min)(bottom, other.bottom)},
	};
}