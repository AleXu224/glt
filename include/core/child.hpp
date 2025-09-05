#pragma once

#include "memory"

namespace squi::core {
	struct Widget;
	struct Element;

	using Child = std::shared_ptr<Widget>;
	using WidgetPtr = std::shared_ptr<Widget>;
	using Context = std::shared_ptr<Element>;
	using ElementPtr = std::shared_ptr<Element>;
}// namespace squi::core