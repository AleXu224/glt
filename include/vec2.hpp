#pragma once
#include "cmath"
#include "glm/glm.hpp"// IWYU pragma: keep
#include "iostream"
#include <limits>
#include <print>


namespace squi {
	struct vec2 {
		float x;
		float y;

		constexpr vec2() noexcept : x(0), y(0) {}
		constexpr vec2(float x, float y) noexcept : x(x), y(y) {}
		constexpr vec2(float xy) noexcept : x(xy), y(xy) {}
		constexpr vec2(const glm::vec2 &vec) noexcept : x(vec.x), y(vec.y) {}
		static constexpr vec2 infinity() noexcept {
			return vec2{
				// I hope the person that created the max macro has a good day...
				(std::numeric_limits<float>::max)(),
				(std::numeric_limits<float>::max)(),
			};
		}

		[[nodiscard]] float length() const noexcept {
			return std::sqrtf((x * x) + (y * y));
		}

		[[nodiscard]] constexpr vec2 operator+(const vec2 &other) const {
			return {x + other.x, y + other.y};
		}
		[[nodiscard]] constexpr vec2 operator-(const vec2 &other) const {
			return {x - other.x, y - other.y};
		}
		[[nodiscard]] constexpr vec2 operator*(const vec2 &other) const {
			return {x * other.x, y * other.y};
		}
		[[nodiscard]] constexpr vec2 operator/(const vec2 &other) const {
			return {x / other.x, y / other.y};
		}

		[[nodiscard]] constexpr vec2 operator-() const {
			return {-x, -y};
		}

		constexpr void operator+=(const vec2 &other) {
			x += other.x;
			y += other.y;
		}
		constexpr void operator-=(const vec2 &other) {
			x -= other.x;
			y -= other.y;
		}
		constexpr void operator*=(const vec2 &other) {
			x *= other.x;
			y *= other.y;
		}
		constexpr void operator/=(const vec2 &other) {
			x /= other.x;
			y /= other.y;
		}

		constexpr bool operator==(const vec2 &other) const {
			return x == other.x && y == other.y;
		}

		[[nodiscard]] constexpr vec2 withX(const float &newX) const {
			return {newX, y};
		}
		[[nodiscard]] constexpr vec2 withY(const float &newY) const {
			return {x, newY};
		}

		[[nodiscard]] constexpr vec2 withXOffset(const float &offset) const {
			return {x + offset, y};
		}
		[[nodiscard]] constexpr vec2 withYOffset(const float &offset) const {
			return {x, y + offset};
		}

		[[nodiscard]] constexpr vec2 rounded() const {
			return {std::roundf(x), std::roundf(y)};
		}

		[[nodiscard]] constexpr operator glm::vec2() const {
			return {x, y};
		}

		// Print
		friend std::ostream &operator<<(std::ostream &os, const vec2 &vec) {
			os << "vec2(" << vec.x << ", " << vec.y << ")";
			return os;
		}

		void print() const {
			std::println("vec2({}, {})", x, y);
		}
	};
}// namespace squi