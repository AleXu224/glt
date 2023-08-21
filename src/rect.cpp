#include "rect.hpp"
#include "algorithm"

using namespace squi;

Rect::Rect(vec2 topLeft, vec2 bottomRight)
	: left(topLeft.x),
	  top(topLeft.y),
	  right(bottomRight.x),
	  bottom(bottomRight.y) {}

Rect Rect::overlap(const Rect &other) const {
	return {
		{(std::max)(left, other.left), (std::max)(top, other.top)},
		{(std::min)(right, other.right), (std::min)(bottom, other.bottom)},
	};
}