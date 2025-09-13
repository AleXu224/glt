#pragma once

#include "margin.hpp"
#include "vec2.hpp"
#include <limits>


namespace squi::core {
	struct BoxConstraints {
		float minWidth = 0.0f;
		float maxWidth = std::numeric_limits<float>::infinity();
		float minHeight = 0.0f;
		float maxHeight = std::numeric_limits<float>::infinity();
		bool shrinkWidth = false;
		bool shrinkHeight = false;

		bool operator==(const BoxConstraints &other) const {
			return minWidth == other.minWidth
				&& maxWidth == other.maxWidth
				&& minHeight == other.minHeight
				&& maxHeight == other.maxHeight
				&& shrinkWidth == other.shrinkWidth
				&& shrinkHeight == other.shrinkHeight;
		}

		void extendToFitMarginPadding(const Margin &margin, const Margin &padding) {
			auto marginSize = margin.getSizeOffset();
			auto paddingSize = padding.getSizeOffset();
			maxWidth = std::max(maxWidth, marginSize.x + paddingSize.x);
			maxHeight = std::max(maxHeight, marginSize.y + paddingSize.y);
		}

		void offsetMaxSize(const vec2 &offset) {
			maxWidth += offset.x;
			maxHeight += offset.y;
		}

		void offsetMinSize(const vec2 &offset) {
			minWidth += offset.x;
			minHeight += offset.y;
		}

		void removePadding(const Margin &padding) {
			auto paddingSize = padding.getSizeOffset();
			offsetMinSize(-paddingSize);
			offsetMaxSize(-paddingSize);
		}

		[[nodiscard]] BoxConstraints withoutPadding(const Margin &padding) const {
			BoxConstraints newConstraints = *this;
			newConstraints.removePadding(padding);
			return newConstraints;
		}

		void clampMaxSizeWith(const BoxConstraints &other) {
			maxWidth = std::min(maxWidth, other.maxWidth);
			maxHeight = std::min(maxHeight, other.maxHeight);
		}

		[[nodiscard]] BoxConstraints withShrink(bool shrinkWidth, bool shrinkHeight) const {
			BoxConstraints newConstraints = *this;
			newConstraints.shrinkWidth = shrinkWidth;
			newConstraints.shrinkHeight = shrinkHeight;
			return newConstraints;
		}

		[[nodiscard]] BoxConstraints withShrinkWidth(bool shrinkWidth) const {
			BoxConstraints newConstraints = *this;
			newConstraints.shrinkWidth = shrinkWidth;
			return newConstraints;
		}

		[[nodiscard]] BoxConstraints withShrinkHeight(bool shrinkHeight) const {
			BoxConstraints newConstraints = *this;
			newConstraints.shrinkHeight = shrinkHeight;
			return newConstraints;
		}

		[[nodiscard]] BoxConstraints copy() const {
			return *this;
		}
	};
}// namespace squi::core