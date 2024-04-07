#pragma once

#include "observer.hpp"
#include "widget.hpp"
#include "color.hpp"

namespace squi {
	namespace Theme {
		struct TextBox {
			static inline Color rest{0xFFFFFF0F};
			static inline Color hover{0xFFFFFF15};
			static inline Color active{0x1E1E1EB2};
			static inline Color disabled{0xFFFFFF0B};

			static inline Color border{0xFFFFFF14};
			static inline Color borderActive{0xFFFFFF12};

			static inline Color bottomBorder{0xFFFFFF8B};
			static inline Color bottomBorderActive{0x60CDFFFF};

			static inline Color text{0xFFFFFFFF};
			static inline Color textDisabled{0xFFFFFF5D};
			static inline Color textHint{0xFFFFFFC8};
		};
	}
	struct TextBox {
		enum class InputState {
			resting,
			hovered,
			focused,
			disabled,
		};
		struct Controller {
			struct ValidatorResponse{
				bool valid = true;
				std::string message = "Invalid input";
			};
			Observable<bool> disable{};
			Observable<bool> focus{};
			std::function<void(std::string_view)> onChange{};
			// Called whenever the input loses focus or when pressing enter
			std::function<void(std::string_view)> onSubmit{};
			std::function<ValidatorResponse(std::string_view)> validator{};
			Observable<std::string_view> updateText{};
			Observable<InputState> stateObserver{};
		};
		// Args
		Widget::Args widget{};
		bool disabled = false;
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
