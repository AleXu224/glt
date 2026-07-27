#pragma once

#include "observer.hpp"
#include "vec2.hpp"
#include <cmath>


namespace squi::core {
	struct Surface {
		float scale = 1.0f;
		float userScaleOffset = 0.0f;
		VoidObservable onScaleChange{};

		void setScale(float newScale) {
			if (scale == newScale) return;
			scale = newScale;
			onScaleChange.notify();
		}

		[[nodiscard]] vec2 toLogical(vec2 physical) const {
			return physical / scale;
		}
		[[nodiscard]] vec2 toPhysical(vec2 logical) const {
			return logical * scale;
		}

		[[nodiscard]] float snapLogical(float v) const {
			return std::roundf(v * scale) / scale;
		}
		[[nodiscard]] vec2 snapLogical(vec2 v) const {
			return {snapLogical(v.x), snapLogical(v.y)};
		}
	};
}// namespace squi::core