#pragma once

#include "vec2.hpp"

namespace squi {
	struct Rect {
		float left;
		float top;
		float right;
		float bottom;

		Rect(vec2 topLeft, vec2 bottomRight);
		[[nodiscard]] static Rect fromPosSize(vec2 position, vec2 size) {
			return {position, position + size};
		}

		[[nodiscard]] float width() const {
			return right - left;
		}
		[[nodiscard]] float height() const {
			return bottom - top;
		}
		[[nodiscard]] vec2 size() const {
			return {width(), height()};
		}

		[[nodiscard]] vec2 getTopLeft() const {
			return {left, top};
		}

		void offset(vec2 offset) {
			left += offset.x;
			right += offset.x;
			top += offset.y;
			bottom += offset.y;
		}

		[[nodiscard]] bool contains(const vec2 &position) const {
			return position.x >= left && position.x < right && position.y >= top && position.y < bottom;
		}

		[[nodiscard]] bool intersects(const Rect &other) const {
			return left <= other.right && right >= other.left && top <= other.bottom && bottom >= other.top;
		}

		Rect &inset(const float &distance) {
			left += distance;
			top += distance;
			right -= distance;
			bottom -= distance;
			return *this;
		}
		Rect &inset(const float &top, const float &right, const float &bottom, const float &left) {
			this->left += left;
			this->top += top;
			this->right -= right;
			this->bottom -= bottom;
			return *this;
		}
		[[nodiscard]] Rect overlap(const Rect &other) const;
	};
}// namespace squi
