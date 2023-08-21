#include "color.hpp"
#include "fontStore.hpp"
#include "window.hpp"
#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <string_view>

using namespace squi;

TEST(FontStore, equality) {
    Window window;
    
    constexpr std::string_view fontPath = R"(C:\Windows\Fonts\arial.ttf)";
    const auto font = FontStore::getFont(fontPath);
    constexpr std::string_view text = "The quick brown fox jumps over the lazy dog";
    constexpr float fontSize = 12;
    
    for (uint32_t i = 0; i < 100; ++i) {
        const auto [width, height] = font->getTextSizeSafe(text, fontSize, static_cast<float>(i));
        const auto [quads, width2, height2] = font->generateQuads(text, fontSize, vec2{0, 0}, Color{}, static_cast<float>(i));

        EXPECT_NE(width, 0);
        EXPECT_NE(height, 0);
        EXPECT_NE(width2, 0);
        EXPECT_NE(height2, 0);

        EXPECT_EQ(width, width2);
        EXPECT_EQ(height, height2);
    }
}