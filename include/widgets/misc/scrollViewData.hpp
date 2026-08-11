#pragma once

#include "observer.hpp"

#include <algorithm>
#include <compare>

namespace squi {
	struct ScrollViewData {
		float viewMainAxis{0.0f};
		float contentMainAxis{0.0f};

		VoidObservable onChange;

		bool operator==(const ScrollViewData &other) const {
			return viewMainAxis == other.viewMainAxis && contentMainAxis == other.contentMainAxis;
		}
		std::partial_ordering operator<=>(const ScrollViewData &other) const {
			if (auto cmp = viewMainAxis <=> other.viewMainAxis; cmp != 0) return cmp;
			return contentMainAxis <=> other.contentMainAxis;
		}

		[[nodiscard]] float clampScroll(float scroll) const {
			if (contentMainAxis <= viewMainAxis) return 0.f;
			return std::clamp(scroll, 0.f, contentMainAxis - viewMainAxis);
		}
	};
};// namespace squi