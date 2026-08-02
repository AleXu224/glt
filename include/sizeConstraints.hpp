#pragma once

#include "optional"

namespace squi {
	struct SizeConstraints {
		std::optional<float> minWidth = std::nullopt;
		std::optional<float> minHeight = std::nullopt;
		std::optional<float> maxWidth = std::nullopt;
		std::optional<float> maxHeight = std::nullopt;

		[[nodiscard]] SizeConstraints withDefaultMinWidth(float value) const;

		[[nodiscard]] SizeConstraints withDefaultMinHeight(float value) const;

		[[nodiscard]] SizeConstraints withDefaultMaxWidth(float value) const;

		[[nodiscard]] SizeConstraints withDefaultMaxHeight(float value) const;

		[[nodiscard]] bool operator==(const SizeConstraints &other) const {
			return minWidth == other.minWidth
				&& minHeight == other.minHeight
				&& maxWidth == other.maxWidth
				&& maxHeight == other.maxHeight;
		}

		inline SizeConstraints withDefaults(const SizeConstraints &defaults) const {
			SizeConstraints ret = *this;
			if (!ret.minWidth) ret.minWidth = defaults.minWidth;
			if (!ret.minHeight) ret.minHeight = defaults.minHeight;
			if (!ret.maxWidth) ret.maxWidth = defaults.maxWidth;
			if (!ret.maxHeight) ret.maxHeight = defaults.maxHeight;
			return ret;
		}
	};
}// namespace squi