#pragma once

#include "core/core.hpp"

namespace squi {
	struct Expander : StatefulWidget {
		// Args
		Key key;
		Args widget;
		Child icon;
		std::variant<std::string, Child> title;
		std::string subtitle;
		bool defaultExpanded = false;
		bool alwaysExpanded = false;
		Child action;
		Child content;

		struct State : WidgetState<Expander> {
			bool expanded = false;

			void initState() override {
				expanded = widget->alwaysExpanded ? true : widget->defaultExpanded;
			}

			Child build(const Element &element) override;
		};
	};
}// namespace squi