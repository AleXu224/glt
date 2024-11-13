#pragma once

#include "ref.hpp"
#include "stateInfo.hpp"
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
		std::function<std::string(float)> formatter{};
		Observable<float> valueUpdater{};
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

		struct State {
			static inline StateInfo<Stateful<Ref<float>, StateImpact::NoImpact>> min{.name = "min"};
			static inline StateInfo<Stateful<Ref<float>, StateImpact::NoImpact>> max{.name = "max"};
			static inline StateInfo<std::reference_wrapper<float>> step{.name = "step"};
		};

		operator squi::Child() const;
	};
}// namespace squi
