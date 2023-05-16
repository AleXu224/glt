#ifndef SQUI_RECT_HPP
#define SQUI_RECT_HPP

#include "vec2.hpp"

namespace squi {
	struct Rect {
		float left;
		float top;
		float right;
		float bottom;

		Rect(vec2 topLeft, vec2 bottomRight);
		[[nodiscard]] inline static Rect fromPosSize(vec2 position, vec2 size) {
			return {position, position + size};
		}

		[[nodiscard]] inline float width() const {
			return right - left;
		}
		[[nodiscard]] inline float height() const {
			return bottom - top;
		}
		[[nodiscard]] vec2 size() const {
			return {width(), height()};
		}

		[[nodiscard]] inline bool contains(const vec2 &position) const {
			return position.x >= left && position.x < right && position.y >= top && position.y < bottom;
		}
		Rect& inset(const float &distance) {
			left += distance;
			top += distance;
			right -= distance;
			bottom -= distance;
			return *this;
		}
		[[nodiscard]] Rect overlap(const Rect &other) const;
	};
}// namespace squi

#endif//SQUI_RECT_HPP
