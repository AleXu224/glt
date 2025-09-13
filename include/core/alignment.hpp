#pragma once

#include "vec2.hpp"

namespace squi::core {
	struct Alignment {
		float horizontal = 0.0f;
		float vertical = 0.0f;

		constexpr Alignment() = default;
		constexpr Alignment(float x, float y) : horizontal(x), vertical(y) {}
		constexpr Alignment(const vec2 &v) : horizontal(v.x), vertical(v.y) {}

		bool operator==(const Alignment &other) const {
			return horizontal == other.horizontal && vertical == other.vertical;
		}

		[[nodiscard]] constexpr vec2 toVec2() const {
			return {horizontal, vertical};
		}

		static const Alignment TopLeft;
		static const Alignment TopCenter;
		static const Alignment TopRight;
		static const Alignment CenterLeft;
		static const Alignment Center;
		static const Alignment CenterRight;
		static const Alignment BottomLeft;
		static const Alignment BottomCenter;
		static const Alignment BottomRight;
	};

	constexpr Alignment Alignment::TopLeft = {0.f, 0.f};
	constexpr Alignment Alignment::TopCenter = {0.5f, 0.0f};
	constexpr Alignment Alignment::TopRight = {1.0f, 0.0f};
	constexpr Alignment Alignment::CenterLeft = {0.0f, 0.5f};
	constexpr Alignment Alignment::Center = {0.5f, 0.5f};
	constexpr Alignment Alignment::CenterRight = {1.0f, 0.5f};
	constexpr Alignment Alignment::BottomLeft = {0.0f, 1.0f};
	constexpr Alignment Alignment::BottomCenter = {0.5f, 1.0f};
	constexpr Alignment Alignment::BottomRight = {1.0f, 1.0f};
}// namespace squi::core