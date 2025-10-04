#include "widgets/animatedBox.hpp"

namespace squi {
	void AnimatedBox::State::assignAndMount(auto &&animated, const auto &value) {
		animated.from = value;
		animated.to = value;
		animated.duration = widget->duration;
		animated.curve = widget->curve;
		animated.mount(this);
	}

	void AnimatedBox::State::assignAndMount(Animated<float> &animated, const std::optional<SizeVariant> &value) {
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

	void AnimatedBox::State::initState() {
		assignAndMount(width, widget->widget.width);
		assignAndMount(height, widget->widget.height);
		assignAndMount(alignment, widget->widget.alignment.value_or(Alignment{}));
		assignAndMount(sizeConstraints, widget->widget.sizeConstraints.value_or(BoxConstraints{}));
		assignAndMount(margin, widget->widget.margin.value_or(Margin{}));
		assignAndMount(padding, widget->widget.padding.value_or(Padding{}));

		assignAndMount(color, widget->color);
		assignAndMount(borderColor, widget->borderColor);
		assignAndMount(borderWidth, widget->borderWidth);
		assignAndMount(borderRadius, widget->borderRadius);
	}

	void AnimatedBox::State::updateOptionalAnimated(auto &&animated, const auto &value) {
		if (value.has_value()) updateAnimated(animated, value.value());
	}

	void AnimatedBox::State::updateAnimated(auto &&animated, const auto &value) {
		animated.duration = widget->duration;
		animated.curve = widget->curve;
		animated = value;
	}

	void AnimatedBox::State::widgetUpdated() {
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

		updateAnimated(color, widget->color);
		updateAnimated(borderColor, widget->borderColor);
		updateAnimated(borderWidth, widget->borderWidth);
		updateAnimated(borderRadius, widget->borderRadius);
	}

	auto AnimatedBox::State::getOptionalValue(const auto &animated, const auto &opt) {
		using T = std::remove_cvref_t<decltype(animated.getValue())>;
		if (opt.has_value()) {
			return std::optional<T>{animated.getValue()};
		}
		return std::optional<T>{};
	}

	std::optional<SizeVariant> AnimatedBox::State::getOptionalValue(const Animated<float> &animated, const std::optional<SizeVariant> &opt) {
		if (opt.has_value()) {
			if (std::holds_alternative<float>(opt.value())) {
				return std::optional<SizeVariant>{animated.getValue()};
			}
			return opt;
		}
		return std::optional<SizeVariant>{};
	}
	Child AnimatedBox::State::build(const Element &) {
		return Box{
			.widget{
				.width = getOptionalValue(width, widget->widget.width),
				.height = getOptionalValue(height, widget->widget.height),
				.alignment = getOptionalValue(alignment, widget->widget.alignment),
				.sizeConstraints = getOptionalValue(sizeConstraints, widget->widget.sizeConstraints),
				.margin = getOptionalValue(margin, widget->widget.margin),
				.padding = getOptionalValue(padding, widget->widget.padding),
			},
			.color = color,
			.borderColor = borderColor,
			.borderWidth = borderWidth,
			.borderRadius = borderRadius,
			.borderPosition = widget->borderPosition,
			.shouldClipContent = widget->shouldClipContent,
			.child = widget->child,
		};
	}
}// namespace squi