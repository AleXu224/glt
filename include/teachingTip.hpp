#pragma once

#include "widgetArgs.hpp"

namespace squi {
	struct TeachingTip {
		// Args
		ChildRef target{};
		std::string_view title{};
		std::string_view message;

		operator squi::Child() const;
	};
}// namespace squi