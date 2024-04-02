#pragma once

#include "widget.hpp"
#include "observer.hpp"

namespace squi {
	struct NumberBox {
        struct Controller{
            Observable<bool> disable{};
            Observable<bool> focus{};
        };
		// Args
		Widget::Args widget{};
        bool disabled = false;
        Controller controller{};

        struct InputState{
			bool hovered = false;
			bool focused = false;
			bool disabled = false;
		};

		struct Storage {
			// Data
            InputState state{};
            Observable<InputState> stateObserver{};
		};

		operator squi::Child() const;
	};
}// namespace squi
