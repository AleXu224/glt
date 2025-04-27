#pragma once

#include "color.hpp"
#include "observer.hpp"
#include "theme.hpp"
#include "widget.hpp"


namespace squi {
	struct TextBox {
		struct Style {
			Color rest{0xFFFFFF0F};
			Color hover{0xFFFFFF15};
			Color active{0x1E1E1EB2};
			Color disabled{0xFFFFFF0B};

			Color border{0xFFFFFF14};
			Color borderActive{0xFFFFFF12};

			Color bottomBorder{0xFFFFFF8B};
			Color bottomBorderActive{ThemeManager::getTheme().accent};

			Color text{0xFFFFFFFF};
			Color textDisabled{0xFFFFFF5D};
			Color textHint{0xFFFFFFC8};
		};
		enum class InputState : uint8_t {
			resting,
			hovered,
			focused,
			disabled,
		};
		struct Controller {
			struct ValidatorResponse {
				bool valid = true;
				std::string message = "Invalid input";
			};
			Observable<bool> disable{};
			Observable<bool> focus{};
			std::function<void(std::string_view)> onChange{};
			// Called whenever the input loses focus or when pressing enter
			std::function<void(std::string_view)> onSubmit{};
			std::function<ValidatorResponse(std::string_view)> validator{};
			Observable<std::string> updateText{};
			Observable<InputState> stateObserver{};
			VoidObservable selectAll{};
		};
		// Args
		Widget::Args widget{};
		bool disabled = false;
		Style style{};
		std::string_view text{};
		Controller controller{};


		struct Storage {
			// Data
			bool hovered = false;
			bool focused = false;
			bool disabled = false;
			[[nodiscard]] InputState getState() const;
			Observable<InputState> stateObserver{};
			Observer<bool> disableObs;
			Observer<bool> focusObs;
			VoidObservable requestOnSubmitCall{};
		};

		operator squi::Child() const;
	};
}// namespace squi
