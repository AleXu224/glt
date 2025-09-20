#pragma once

#include "core/animated.hpp"
#include "widgets/box.hpp"


namespace squi {
	struct AnimatedBox : StatefulWidget {
		// Args
		Key key;
		Args widget;

		std::chrono::milliseconds duration = 200ms;
		std::function<float(float)> curve = core::Curve::easeOutCubic;

		Color color{0xFFFFFFFF};
		Color borderColor{0x000000FF};
		BorderWidth borderWidth{0.0f};
		BorderRadius borderRadius{0.0f};
		Box::BorderPosition borderPosition{Box::BorderPosition::inset};
		bool shouldClipContent = true;
		Child child;

		struct State : WidgetState<AnimatedBox> {
			Animated<float> width{};
			Animated<float> height{};
			Animated<Alignment> alignment{};
			Animated<BoxConstraints> sizeConstraints{};
			Animated<Margin> margin{};
			Animated<Margin> padding{};

			Animated<Color> color{};
			Animated<Color> borderColor{};
			Animated<BorderWidth> borderWidth{};
			Animated<BorderRadius> borderRadius{};

			void assignAndMount(auto &&animated, const auto &value) {
				animated.from = value;
				animated.to = value;
				animated.duration = widget->duration;
				animated.curve = widget->curve;
				animated.mount(this);
			}

			void assignAndMount(Animated<float> &animated, const std::optional<SizeVariant> &value) {
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

			void initState() override {
				assignAndMount(width, widget->widget.width);
				assignAndMount(height, widget->widget.height);
				assignAndMount(alignment, widget->widget.alignment.value_or({}));
				assignAndMount(sizeConstraints, widget->widget.sizeConstraints.value_or({}));
				assignAndMount(margin, widget->widget.margin.value_or({}));
				assignAndMount(padding, widget->widget.padding.value_or({}));

				assignAndMount(color, widget->color);
				assignAndMount(borderColor, widget->borderColor);
				assignAndMount(borderWidth, widget->borderWidth);
				assignAndMount(borderRadius, widget->borderRadius);
			}

			void updateOptionalAnimated(auto &&animated, const auto &value) {
				if (value.has_value()) updateAnimated(animated, value.value());
			}

			void updateAnimated(auto &&animated, const auto &value) {
				animated.duration = widget->duration;
				animated.curve = widget->curve;
				animated = value;
			}

			void widgetUpdated() override {
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

			auto getOptionalValue(const auto &animated, const auto &opt) {
				using T = std::remove_cvref_t<decltype(animated.getValue())>;
				if (opt.has_value()) {
					return std::optional<T>{animated.getValue()};
				}
				return std::optional<T>{};
			}

			static std::optional<SizeVariant> getOptionalValue(const Animated<float> &animated, const std::optional<SizeVariant> &opt) {
				if (opt.has_value()) {
					if (std::holds_alternative<float>(opt.value())) {
						return std::optional<SizeVariant>{animated.getValue()};
					}
					return opt;
				}
				return std::optional<SizeVariant>{};
			}

			Child build(const Element &) override {
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
		};
	};
}// namespace squi