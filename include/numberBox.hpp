#pragma once

#include "textBox.hpp"
#include "widget.hpp"


namespace squi {
	struct NumberBox {
		// Args
		Widget::Args widget{};
		float value = 0;
		float min = std::numeric_limits<float>::lowest();
		float max = std::numeric_limits<float>::max();
		float step = 1.f;
		bool disabled = false;
		std::function<void(float)> onChange{};
		std::function<TextBox::Controller::ValidatorResponse(float)> validator{};
		TextBox::Controller controller{};

		struct Storage {
			float value;
			float min;
			float max;
			float step;
			bool focused = false;
			std::function<void(float)> onChange{};

			void clampValue();
		};

		operator squi::Child() const;
	};
}// namespace squi
