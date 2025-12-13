#pragma once

#include "core/core.hpp"
#include "widgets/button.hpp"
#include "widgets/textInput.hpp"


namespace squi {
	struct TextBox : StatefulWidget {
		// Args
		Key key;
		Args widget{};
		bool disabled = false;
		TextInput::Controller controller{};
		std::function<std::optional<std::string>(const std::string &)> validator;

		struct State : WidgetState<TextBox> {
			Button::ButtonStatus status = Button::ButtonStatus::resting;
			bool active = false;
			bool hovered = false;
			std::string errorMessage;

			void updateStatus();

			void widgetUpdated() override {
				updateStatus();
			}

			Child build(const Element &) override;
		};
	};
}// namespace squi