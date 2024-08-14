#include "color.hpp"
#include "fontStore.hpp"
#include "window.hpp"
#include <catch2/catch_test_macros.hpp>
#include <string_view>

using namespace squi;

TEST_CASE("FontStore equality") {
	const auto &font = FontStore::defaultFont;
	constexpr std::string_view text = "The quick brown fox jumps over the lazy dog";
	constexpr float fontSize = 12;

	for (uint32_t i = 0; i < 100; ++i) {
		const auto [width, height] = font->getTextSizeSafe(text, fontSize, static_cast<float>(i));
		const auto [quads, width2, height2] = font->generateQuads(text, fontSize, vec2{0, 0}, Color{}, static_cast<float>(i));

		REQUIRE(width != 0);
		REQUIRE(height != 0);
		REQUIRE(width2 != 0);
		REQUIRE(height2 != 0);

		REQUIRE(width == width2);
		REQUIRE(height == height2);
	}
}