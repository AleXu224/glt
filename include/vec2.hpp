#ifndef SQUI_VEC2_HPP
#define SQUI_VEC2_HPP

#include "AnimatedFloat.hpp"
#include "DirectXMath.h"

namespace squi {
	struct vec2 {
		float x;
		float y;

		constexpr vec2() noexcept : x(0), y(0) {}
		vec2(float x, float y) noexcept : x(x), y(y) {}
		vec2(float xy) noexcept : x(xy), y(xy) {}

		[[nodiscard]] inline vec2 operator+(const vec2 &other) const {
			return {x + other.x, y + other.y};
		}
		[[nodiscard]] inline vec2 operator-(const vec2 &other) const {
			return {x - other.x, y - other.y};
		}
		[[nodiscard]] inline vec2 operator*(const vec2 &other) const {
			return {x * other.x, y * other.y};
		}
		[[nodiscard]] inline vec2 operator/(const vec2 &other) const {
			return {x / other.x, y / other.y};
		}

		inline void operator+=(const vec2 &other) {
			x += other.x;
			y += other.y;
		}
		inline void operator-=(const vec2 &other) {
			x -= other.x;
			y -= other.y;
		}
		inline void operator*=(const vec2 &other) {
			x *= other.x;
			y *= other.y;
		}
		inline void operator/=(const vec2 &other) {
			x /= other.x;
			y /= other.y;
		}

		inline bool operator==(const vec2 &other) const {
			return x == other.x && y == other.y;
		}

		[[nodiscard]] inline vec2 withX(const float &newX) const {
			return {newX, y};
		}
		[[nodiscard]] inline vec2 withY(const float &newY) const {
			return {x, newY};
		}

		[[nodiscard]] inline vec2 withXOffset(const float &offset) const {
			return {x + offset, y};
		}
		[[nodiscard]] inline vec2 withYOffset(const float &offset) const {
			return {x, y + offset};
		}

		[[nodiscard]] inline vec2 rounded() const {
			return {std::round(x), std::round(y)};
		}

		[[nodiscard]] inline operator DirectX::XMFLOAT2() const {
			return {x, y};
		}
	};
}// namespace squi

#endif