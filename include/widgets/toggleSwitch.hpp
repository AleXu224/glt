#pragma once

#include "core/core.hpp"
#include "widgets/button.hpp"


namespace squi {
	struct ToggleSwitch : StatefulWidget {
		// Args
		enum class StatePosition {
			Left,
			Right,
			None,
		};

		// Args
		Key key;
		bool active;
		bool disabled = false;
		StatePosition statePosition = StatePosition::Right;
		std::function<void(bool)> onToggle;

		struct State : WidgetState<ToggleSwitch> {
			static Button::Theme getTheme();

			static Button::Theme getActiveTheme();

			Button::ButtonStatus status = Button::ButtonStatus::resting;
			bool active = false;
			bool hovered = false;

			void updateStatus();

			Child build(const Element &element) override;
		};
	};
}// namespace squi