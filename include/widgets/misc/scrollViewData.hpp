#pragma once

#include <algorithm>
#include <compare>

namespace squi {
	struct ScrollViewData {
		float viewMainAxis{0.0f};
		float contentMainAxis{0.0f};

		std::partial_ordering operator<=>(const ScrollViewData &) const = default;

		[[nodiscard]] float clampScroll(float scroll) const {
			if (contentMainAxis <= viewMainAxis) return 0.f;
			return std::clamp(scroll, 0.f, contentMainAxis - viewMainAxis);
		}
	};
};// namespace squi