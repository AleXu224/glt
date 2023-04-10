#ifndef SQUI_VEC2_HPP
#define SQUI_VEC2_HPP

#include "AnimatedFloat.hpp"
#include "DirectXMath.h"

namespace squi {
	struct vec2 {
		float x;
		float y;

		vec2() : x(0), y(0) {}
		vec2(float x, float y) : x(x), y(y) {}
		vec2(float xy) : x(xy), y(xy) {}

		[[nodiscard]] vec2 operator+(const vec2 &other) const;
		[[nodiscard]] vec2 operator-(const vec2 &other) const;
		[[nodiscard]] vec2 operator*(const vec2 &other) const;
		[[nodiscard]] vec2 operator/(const vec2 &other) const;

		void operator+=(const vec2 &other);
		void operator-=(const vec2 &other);
		void operator*=(const vec2 &other);
		void operator/=(const vec2 &other);

		bool operator==(const vec2 &other) const;

		[[nodiscard]] vec2 withX(const float &newX) const;
		[[nodiscard]] vec2 withY(const float &newY) const;

		[[nodiscard]] vec2 withXOffset(const float &offset) const;
		[[nodiscard]] vec2 withYOffset(const float &offset) const;

		[[nodiscard]] operator DirectX::XMFLOAT2() const;
	};
}// namespace squi

#endif