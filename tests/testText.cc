#include "algorithm"
#include "text.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string_view>


using namespace squi;

TEST_CASE("Text equality") {
	std::array<std::string_view, 2> textTestCases{
		"The quick brown fox jumps over the lazy dog",
		"Charged Attack DMG"
	};

	for (const auto &textCase: textTestCases) {
		std::shared_ptr<Text::Impl> text = std::static_pointer_cast<Text::Impl>(Child{Text{
			.text = textCase,
			.fontSize = 12,
		}});

		for (uint32_t i = 0; i < 100; ++i) {
			const vec2 maxSize = {static_cast<float>(i), static_cast<float>(i)};
			const vec2 layoutSize = text->layout(maxSize, {0, 0}, Widget::ShouldShrink{true, true}, true);
			const auto [minWidth, minHeight] = text->getTextSize(textCase);

			REQUIRE(((std::min)(static_cast<float>(minWidth), maxSize.x)) == layoutSize.x);
			REQUIRE(((std::min)(static_cast<float>(minHeight), maxSize.y)) == layoutSize.y);
		}


		std::shared_ptr<Text::Impl> text2 = std::static_pointer_cast<Text::Impl>(Child{Text{
			.text = textCase,
			.fontSize = 12,
			.lineWrap = true,
		}});

		for (uint32_t i = 0; i < 100; ++i) {
			const vec2 maxSize = {static_cast<float>(i), static_cast<float>(i)};
			const vec2 layoutSize = text2->layout(maxSize, {0, 0}, Widget::ShouldShrink{true, true}, true);
			const auto [minWidth, minHeight] = text2->getTextSize(textCase);

			REQUIRE(((std::min)(static_cast<float>(minWidth), maxSize.x)) == layoutSize.x);
			REQUIRE(((std::min)(static_cast<float>(minHeight), maxSize.y)) == layoutSize.y);
		}
	}
}