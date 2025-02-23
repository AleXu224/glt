#pragma once

#include "glm/fwd.hpp"

namespace squi {
	struct BorderRadius {
		float topLeft{0};
		float topRight{0};
		float bottomRight{0};
		float bottomLeft{0};

		BorderRadius() = default;
		BorderRadius(float all);
		BorderRadius(float topLeft, float topRight, float bottomRight, float bottomLeft);

		[[nodiscard]] bool operator==(const BorderRadius &other) const {
			return topLeft == other.topLeft
				&& topRight == other.topRight
				&& bottomRight == other.bottomRight
				&& bottomLeft == other.bottomLeft;
		}

		static BorderRadius TopLeft(float value);
		static BorderRadius TopRight(float value);
		static BorderRadius BottomRight(float value);
		static BorderRadius BottomLeft(float value);

		[[nodiscard]] BorderRadius withTopLeft(float value);
		[[nodiscard]] BorderRadius withTopRight(float value);
		[[nodiscard]] BorderRadius withBottomRight(float value);
		[[nodiscard]] BorderRadius withBottomLeft(float value);

		static BorderRadius Top(float value);
		static BorderRadius Bottom(float value);
		static BorderRadius Left(float value);
		static BorderRadius Right(float value);

		[[nodiscard]] BorderRadius withTop(float value);
		[[nodiscard]] BorderRadius withBottom(float value);
		[[nodiscard]] BorderRadius withLeft(float value);
		[[nodiscard]] BorderRadius withRight(float value);

		operator glm::vec4() const;
	};
}// namespace squi