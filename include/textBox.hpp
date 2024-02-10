#pragma once

#include "color.hpp"
#include "widget.hpp"

namespace squi {
	struct TextBox {
		struct Theme {
			Color rest{0xFFFFFF0F};
			Color hover{0xFFFFFF15};
			Color active{0x1E1E1EB2};
			Color disabled{0xFFFFFF0B};

			Color border{0xFFFFFF14};
			Color borderActive{0xFFFFFF12};

			Color bottomBorder{0xFFFFFF8B};
			Color bottomBorderActive{0x60CDFFFF};

			Color text{0xFFFFFFFF};
			Color textDisabled{0xFFFFFF5D};
			Color textHint{0xFFFFFFC8};
		};
		static Theme theme;
		// Args
		Widget::Args widget{};

		struct Storage {
			// Data
			enum class State {
				rest,
				hover,
				active,
				disabled,
			};
			State state = State::rest;
			bool changed = false;
		};

		operator Child() const;
	};
}// namespace squi