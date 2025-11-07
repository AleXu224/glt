#pragma once

#include "core/animated.hpp"
#include "fontStore.hpp"
#include "text/provider.hpp"


namespace squi {
	struct AnimatedText : StatefulWidget {
		// Args
		Key key;
		Args widget{};

		std::chrono::milliseconds duration = 200ms;
		std::function<float(float)> curve = core::Curve::easeOutCubic;

		std::string text;
		float fontSize{14.0f};
		bool lineWrap{false};
		std::variant<FontProvider, std::shared_ptr<FontStore::Font>> font = FontStore::defaultFont;
		Color color = Color::white;

		struct State : WidgetState<AnimatedText> {
			Animated<float> width{};
			Animated<float> height{};
			Animated<Alignment> alignment{};
			Animated<BoxConstraints> sizeConstraints{};
			Animated<Margin> margin{};
			Animated<Margin> padding{};

			Animated<float> fontSize{};
			Animated<Color> color{};

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
