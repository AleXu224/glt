#pragma once

#include <glm/fwd.hpp>

namespace squi {
	struct BorderWidth {
		float top{0};
		float right{0};
		float bottom{0};
		float left{0};

		BorderWidth() = default;
		BorderWidth(float all);
		BorderWidth(float top, float right, float bottom, float left);

		static BorderWidth Top(float width);
		static BorderWidth Right(float width);
		static BorderWidth Bottom(float width);
		static BorderWidth Left(float width);

		[[nodiscard]] BorderWidth withTop(float width) const;
		[[nodiscard]] BorderWidth withRight(float width) const;
		[[nodiscard]] BorderWidth withBottom(float width) const;
		[[nodiscard]] BorderWidth withLeft(float width) const;

		operator glm::vec4() const;

		bool operator==(const BorderWidth &other) const;
	};
}// namespace squi