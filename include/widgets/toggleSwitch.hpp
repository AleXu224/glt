#pragma once

#include "button.hpp"
#include "widget.hpp"

namespace squi {
	struct ToggleSwitch {
		struct ToggleSwitchState {
			ButtonState state = ButtonState::resting;
			bool active = false;
		};
		enum class StatePosition {
			disabled,
			before,
			after,
		};
		// Args
		squi::Widget::Args widget{};
		StatePosition statePosition = ToggleSwitch::StatePosition::after;
		bool disabled = false;
		bool active = false;
		std::function<void(bool)> onSwitch{};

		struct State {
			static inline StateInfo<bool> disabled{.name = "disabled"};
			static inline StateInfo<bool> active{.name = "active"};
			static inline StateInfo<ButtonState> state{.name = "state"};
			static inline StateInfo<Observable<ToggleSwitchState>> stateEvent{.name = "stateEvent"};
		};

		operator squi::Child() const;
	};
}// namespace squi
