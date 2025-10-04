#include "widgets/animatedText.hpp"

#include "widgets/text.hpp"

namespace squi {

	void AnimatedText::State::assignAndMount(auto &&animated, const auto &value) {
		animated.from = value;
		animated.to = value;
		animated.duration = widget->duration;
		animated.curve = widget->curve;
		animated.mount(this);
	}

	void AnimatedText::State::assignAndMount(Animated<float> &animated, const std::optional<SizeVariant> &value) {
		if (value.has_value()) {
			if (std::holds_alternative<float>(value.value())) {
				animated.from = std::get<float>(value.value());
				animated.to = std::get<float>(value.value());
			} else {
				animated.from = 0.f;
				animated.to = 0.f;
			}
		}
		animated.duration = widget->duration;
		animated.curve = widget->curve;
		animated.mount(this);
	}

	void AnimatedText::State::initState() {
		assignAndMount(fontSize, widget->fontSize);
		assignAndMount(color, widget->color);

		assignAndMount(width, widget->widget.width);
		assignAndMount(height, widget->widget.height);
		assignAndMount(alignment, widget->widget.alignment.value_or(Alignment{}));
		assignAndMount(sizeConstraints, widget->widget.sizeConstraints.value_or(BoxConstraints{}));
		assignAndMount(margin, widget->widget.margin.value_or(Margin{}));
		assignAndMount(padding, widget->widget.padding.value_or(Padding{}));
	}

	void AnimatedText::State::updateOptionalAnimated(auto &&animated, const auto &value) {
		if (value.has_value()) updateAnimated(animated, value.value());
	}

	void AnimatedText::State::updateAnimated(auto &&animated, const auto &value) {
		animated.duration = widget->duration;
		animated.curve = widget->curve;
		animated = value;
	}

	void AnimatedText::State::widgetUpdated() {
		if (widget->widget.width.has_value() && std::holds_alternative<float>(widget->widget.width.value())) {
			updateAnimated(width, std::get<float>(widget->widget.width.value()));
		}
		if (widget->widget.height.has_value() && std::holds_alternative<float>(widget->widget.height.value())) {
			updateAnimated(height, std::get<float>(widget->widget.height.value()));
		}

		updateOptionalAnimated(alignment, widget->widget.alignment);
		updateOptionalAnimated(sizeConstraints, widget->widget.sizeConstraints);
		updateOptionalAnimated(margin, widget->widget.margin);
		updateOptionalAnimated(padding, widget->widget.padding);

		updateAnimated(fontSize, widget->fontSize);
		updateAnimated(color, widget->color);
	}

	auto AnimatedText::State::getOptionalValue(const auto &animated, const auto &opt) {
		using T = std::remove_cvref_t<decltype(animated.getValue())>;
		if (opt.has_value()) {
			return std::optional<T>{animated.getValue()};
		}
		return std::optional<T>{};
	}

	std::optional<SizeVariant> AnimatedText::State::getOptionalValue(const Animated<float> &animated, const std::optional<SizeVariant> &opt) {
		if (opt.has_value()) {
			if (std::holds_alternative<float>(opt.value())) {
				return std::optional<SizeVariant>{animated.getValue()};
			}
			return opt;
		}
		return std::optional<SizeVariant>{};
	}

	Child AnimatedText::State::build(const Element &) {
		return Text{
			.widget{
				.width = getOptionalValue(width, widget->widget.width),
				.height = getOptionalValue(height, widget->widget.height),
				.alignment = getOptionalValue(alignment, widget->widget.alignment),
				.sizeConstraints = getOptionalValue(sizeConstraints, widget->widget.sizeConstraints),
				.margin = getOptionalValue(margin, widget->widget.margin),
				.padding = getOptionalValue(padding, widget->widget.padding),
			},
			.text = widget->text,
			.fontSize = fontSize,
			.lineWrap = widget->lineWrap,
			.font = widget->font,
			.color = color,
		};
	}
}// namespace squi