#pragma once

namespace squi::core::Curve {
	inline float linear(float t) {
		return t;
	}

	inline float easeInCubic(float t) {
		return t * t * t;
	}

	inline float easeOutCubic(float t) {
		return 1.0f - easeInCubic(1.0f - t);
	}

	inline float easeInOutCubic(float t) {
		return t < 0.5f ? easeInCubic(t * 2.0f) / 2.0f : 1.0f - (easeInCubic((1.0f - t) * 2.0f) / 2.0f);
	}
}// namespace squi::core::Curve
