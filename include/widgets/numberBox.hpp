#pragma once

#include "core/core.hpp"
#include "widgets/textInput.hpp"
#include <functional>
#include <limits>

namespace squi {
	struct NumberBox : StatefulWidget {
		// Args
		Key key;
		Args widget{};
		bool disabled = false;
		double value = 0.0;
		double min = -std::numeric_limits<double>::infinity();
		double max = std::numeric_limits<double>::infinity();
		int precision = 0;
		std::function<void(double)> onChange;

		struct State : WidgetState<NumberBox> {
			TextInput::Controller controller;
			Observer<const std::string &> observer;
			double lastKnownValue = 0.0;

			void initState() override;
			void widgetUpdated() override;
			Child build(const Element &) override;
		};
	};
}// namespace squi