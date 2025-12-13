#pragma once

#include "core/core.hpp"
#include "observer.hpp"

namespace squi {
	struct Dialog : StatefulWidget {
		// Args
		Key key;
		Args widget{};
		float width = 540.f;
		VoidObservable closeEvent{};
		std::string title;
		Child content{};
		Children buttons{};

		struct State : WidgetState<Dialog> {
			Child build(const Element &element) override;
		};
	};
}// namespace squi