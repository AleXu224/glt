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
	};
}