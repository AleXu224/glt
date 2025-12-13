#include "widgets/numberBox.hpp"
#include "widgets/textBox.hpp"
#include <cmath>
#include <format>

namespace squi {
	void NumberBox::State::initState() {
		lastKnownValue = widget->value;
		controller = TextInput::Controller(std::format("{:.{}f}", widget->value, widget->precision));
		observer = controller.getTextObserver([this](const std::string &text) {
			if (text.empty() || text == "-") return;
			double val = 0.0;
			auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), val);
			if (ec == std::errc() && ptr == text.data() + text.size()) {
				if (val >= widget->min && val <= widget->max) {
					if (widget->onChange) widget->onChange(val);
				}
			}
		});
	}

	void NumberBox::State::widgetUpdated() {
		if (std::abs(widget->value - lastKnownValue) > 1e-6) {
			lastKnownValue = widget->value;

			// Check if current text already represents this value
			double currentTextVal = 0.0;
			bool textMatches = false;

			auto text = controller.getText();
			auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), currentTextVal);
			if (ec == std::errc() && ptr == text.data() + text.size()) {
				if (std::abs(currentTextVal - widget->value) < 1e-6) {
					textMatches = true;
				}
			}

			if (!textMatches) {
				controller.setText(std::format("{:.{}f}", widget->value, widget->precision));
			}
		}
	}

	core::Child NumberBox::State::build(const Element &) {
		return TextBox{
			.widget = widget->widget,
			.disabled = widget->disabled,
			.controller = controller,
			.validator = [this](const std::string &text) -> std::optional<std::string> {
				if (text.empty()) return std::nullopt;
				if (text == "-") return std::nullopt;

				try {
					size_t pos = 0;
					double val = std::stod(text, &pos);
					if (pos != text.size()) return "Invalid number";
					if (val < widget->min) return "Value too low";
					if (val > widget->max) return "Value too high";
				} catch (...) {
					return "Invalid number";
				}
				return std::nullopt;
			},
		};
	}
}// namespace squi