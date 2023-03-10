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
		[[nodiscard]] static Rect fromPosSize(vec2 position, vec2 size);

		[[nodiscard]] float width() const;
		[[nodiscard]] float height() const;
		[[nodiscard]] vec2 size() const;


		[[nodiscard]] bool contains(const vec2 &position) const;
		[[nodiscard]] Rect overlap(const Rect &other) const;
	};
}// namespace squi

#endif//SQUI_RECT_HPP
