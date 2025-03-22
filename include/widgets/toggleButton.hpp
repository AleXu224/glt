#pragma once

#include "button.hpp"
#include "stateInfo.hpp"
#include "widget.hpp"


namespace squi {
	struct ToggleButton {
		// Args
		squi::Widget::Args widget{};
		std::string_view text;
		bool disabled = false;
		bool active = false;
		std::function<void(ButtonStyle &, bool)> modifyStyle{};
		std::function<void(bool)> onSwitch{};

		struct State {
			static inline StateInfo<bool> active{.name = "active"};
			static inline StateInfo<Observable<bool>> stateEvent{.name = "toggleButtonStateEvent"};
		};

		operator squi::Child() const;
	};
}// namespace squi