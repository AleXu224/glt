#pragma once

#include "core/core.hpp"
#include "widgets/button.hpp"
#include "widgets/textInput.hpp"

#include <functional>
#include <string>
#include <vector>


namespace squi {
	struct TextBox : StatefulWidget {
		// Args
		Key key;
		Args widget{};
		bool disabled = false;
		bool multiline = false;
		TextInput::Controller controller{};
		std::function<std::optional<std::string>(const std::string &)> validator;
		std::function<void(const std::string &)> onSubmit;

		struct Suggestion {
			std::string label;
			std::function<void()> onSelect;
		};
		std::vector<Suggestion> suggestions;

		struct State : WidgetState<TextBox> {
			Button::ButtonStatus status = Button::ButtonStatus::resting;
			bool active = false;
			bool hovered = false;
			std::string errorMessage;

			Key overlayKey = GlobalKey{};
			Key targetKey = GlobalKey{};
			bool overlayShown = false;
			int64_t menuSelectedIndex = -1;

			~State();

			void updateStatus();
			void updateOverlay();
			void handleSubmit(const std::string &text);
			void closeOverlay();

			[[nodiscard]] bool isMouseButtonDown();

			void widgetUpdated() override;

			Child build(const Element &) override;
		};
	};
}// namespace squi
