#pragma once

#include "stateInfo.hpp"
#include "widget.hpp"

namespace squi {
	struct Slider {
		// Args
		squi::Widget::Args widget{};
		float value = 0.5f;
		float minVal = 0.f;
		float maxVal = 1.f;
		std::optional<float> step = std::nullopt;
		std::function<void(float)> valueChanged{};
		struct State {
			static inline StateInfo<Stateful<float, StateImpact::RelayoutNeeded>> minVal{.name = "minVal"};
			static inline StateInfo<Stateful<float, StateImpact::RelayoutNeeded>> maxVal{.name = "maxVal"};
			static inline StateInfo<Stateful<std::optional<float>, StateImpact::RelayoutNeeded>> step{.name = "step"};
			static inline StateInfo<Stateful<float, StateImpact::RelayoutNeeded>> value{.name = "value"};
		};

		operator squi::Child() const;

	private:
		static float calcValue(float min, float max, std::optional<float> step, float val);
	};
}// namespace squi