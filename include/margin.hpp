#ifndef SQUI_MARGIN_HPP
#define SQUI_MARGIN_HPP

#include "vec2.hpp"

namespace squi {
    /**
     * @brief A structure that represents an area around a rectangle.
     * Can be used to represent padding or margin.
     * 
     */
	struct Margin {
		AnimatedFloat top;
		AnimatedFloat right;
		AnimatedFloat bottom;
		AnimatedFloat left;

	public:
        Margin() : top(0), right(0), bottom(0), left(0) {}
        Margin(float top, float right, float bottom, float left) : top(top), right(right), bottom(bottom), left(left) {}
        Margin(float x, float y) : top(y), right(x), bottom(y), left(x) {}
        Margin(float all) : top(all), right(all), bottom(all), left(all) {}
        Margin(const vec2 &vec) : top(vec.y), right(vec.x), bottom(vec.y), left(vec.x) {}
        Margin(const vec2 &topLeft, const vec2 &bottomRight) : top(topLeft.y), right(bottomRight.x), bottom(bottomRight.y), left(topLeft.x) {}

        [[nodiscard]] Margin operator+(const Margin &other) const;
        [[nodiscard]] Margin operator-(const Margin &other) const;
        [[nodiscard]] Margin operator*(const Margin &other) const;
        [[nodiscard]] Margin operator/(const Margin &other) const;

        void operator+=(const Margin &other);
        void operator-=(const Margin &other);
        void operator*=(const Margin &other);
        void operator/=(const Margin &other);

        [[nodiscard]] Margin withTop(const float &newTop) const;
        [[nodiscard]] Margin withRight(const float &newRight) const;
        [[nodiscard]] Margin withBottom(const float &newBottom) const;
        [[nodiscard]] Margin withLeft(const float &newLeft) const;

        [[nodiscard]] Margin withTopOffset(const float &offset) const;
        [[nodiscard]] Margin withRightOffset(const float &offset) const;
        [[nodiscard]] Margin withBottomOffset(const float &offset) const;
        [[nodiscard]] Margin withLeftOffset(const float &offset) const;

        [[nodiscard]] Margin withHorizontal(const float &newHorizontal) const;
        [[nodiscard]] Margin withVertical(const float &newVertical) const;

        [[nodiscard]] Margin withHorizontalOffset(const float &offset) const;
        [[nodiscard]] Margin withVerticalOffset(const float &offset) const;

		[[nodiscard]] vec2 getPositionOffset() const;
		[[nodiscard]] vec2 getSizeOffset() const;
	};
}// namespace squi

#endif