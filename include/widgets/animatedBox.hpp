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

			void assignAndMount(auto &&animated, const auto &value);

			void assignAndMount(Animated<float> &animated, const std::optional<SizeVariant> &value);

			void initState() override;

			void updateOptionalAnimated(auto &&animated, const auto &value);

			void updateAnimated(auto &&animated, const auto &value);

			void widgetUpdated() override;

			auto getOptionalValue(const auto &animated, const auto &opt);

			static std::optional<SizeVariant> getOptionalValue(const Animated<float> &animated, const std::optional<SizeVariant> &opt);

			Child build(const Element &) override;
		};
	};
}// namespace squi
