#pragma once

#include "core/core.hpp"
#include "theme.hpp"

namespace squi {
	struct ThemeOverride : InheritedWidget<ThemeOverride> {
		// Args
		Key key;
		Theme theme;
		Child child;

		struct Context {
			const ThemeOverride *widget;
		};
	};
}// namespace squi