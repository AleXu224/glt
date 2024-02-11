#include "text.hpp"
#include "window.hpp"
#include "gtest/gtest.h"
#include <memory>
#include "algorithm"
#include <string_view>

using namespace squi;

TEST(Text, equality) {
    Window window{};
    window.state.root = &window;
    std::array<std::string_view, 2> textTestCases {
        "The quick brown fox jumps over the lazy dog",
        "Charged Attack DMG"
    };

    for (const auto& textCase : textTestCases) {
		std::shared_ptr<Text::Impl> text = std::static_pointer_cast<Text::Impl>(Child{Text{
			.text = textCase,
			.fontSize = 12,
		}});
        text->state.root = &window;
        text->state.parent = &window;

		for (uint32_t i = 0; i < 100; ++i) {
            const vec2 maxSize = {static_cast<float>(i), static_cast<float>(i)};
            const vec2 layoutSize = text->layout(maxSize, {0, 0}, Widget::ShouldShrink{true, true});
            const auto [minWidth, minHeight] = text->getTextSize(textCase);

            EXPECT_EQ(((std::min)(static_cast<float>(minWidth), maxSize.x)), layoutSize.x) << "i = " << i << ", text = " << textCase;
            EXPECT_EQ(((std::min)(static_cast<float>(minHeight), maxSize.y)), layoutSize.y) << "i = " << i << ", text = " << textCase;
        }


		std::shared_ptr<Text::Impl> text2 = std::static_pointer_cast<Text::Impl>(Child{Text{
			.text = textCase,
			.fontSize = 12,
			.lineWrap = true,
		}});
		text2->state.root = &window;
		text2->state.parent = &window;

		for (uint32_t i = 0; i < 100; ++i) {
            const vec2 maxSize = {static_cast<float>(i), static_cast<float>(i)};
			const vec2 layoutSize = text2->layout(maxSize, {0, 0}, Widget::ShouldShrink{true, true});
			const auto [minWidth, minHeight] = text2->getTextSize(textCase);

			EXPECT_EQ(((std::min)(static_cast<float>(minWidth), maxSize.x)), layoutSize.x) << "i = " << i << ", text = " << textCase;
			EXPECT_EQ(((std::min)(static_cast<float>(minHeight), maxSize.y)), layoutSize.y) << "i = " << i << ", text = " << textCase;
		}
    }
}