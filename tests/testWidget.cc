#include "container.hpp"
#include "widget.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace squi;

TEST_CASE("Widget layout") {
	{
		Widget widget{
			Widget::Args{
				.width = 100.f,
				.height = 100.f,
			},
			Widget::FlagsArgs::Default(),
		};

		const vec2 maxSize{1000.f, 1000.f};

		vec2 expectedSize{100.f, 100.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);// "Widget should return its size when the size is fixed";

		const vec2 smallerMaxSize{50.f, 50.f};
		expectedSize = {50.f, 50.f};
		REQUIRE(widget.layout(smallerMaxSize, {0, 0}, {}, true) == expectedSize);// "Widget should return the maxSize when the size is larger than the maxSize";

		widget.state.width = Size::Expand;
		widget.state.height = Size::Expand;
		expectedSize = maxSize;
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);// "Widget should return the maxSize when the size is expand";

		widget.state.margin = 10.f;
		widget.state.padding = 10.f;
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);

		widget.state.sizeConstraints.maxHeight = 50.f;
		widget.state.sizeConstraints.maxWidth = 50.f;
		widget.state.margin = 0.f;
		widget.state.padding = 0.f;
		expectedSize = {50.f, 50.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);// "Widget should still follow the constraints when the size is expand";

		widget.state.margin = 10.f;
		widget.state.padding = 10.f;
		expectedSize = {70.f, 70.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);
		widget.state.margin = 0.f;
		widget.state.padding = 0.f;

		widget.state.sizeConstraints.maxHeight = std::nullopt;
		widget.state.sizeConstraints.maxWidth = std::nullopt;
		widget.state.width = Size::Shrink;
		widget.state.height = Size::Shrink;
		expectedSize = {0.f, 0.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);// "Widget has no content, padding or margin so the size should be 0";

		widget.state.margin = 10.f;
		widget.state.padding = 10.f;
		expectedSize = {40.f, 40.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);// "Widget has a margin and padding so the size should be 2 * margin + 2 * padding";
		widget.state.margin = 0.f;
		widget.state.padding = 0.f;

		widget.state.sizeConstraints.minHeight = 50.f;
		widget.state.sizeConstraints.minWidth = 50.f;
		expectedSize = {50.f, 50.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);// "Widget should still follow the constraints when the size is shrink";

		widget.state.margin = 10.f;
		expectedSize = {70.f, 70.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);// "Widget has a margin so the size should be 2 * margin + size";

		widget.state.padding = 10.f;
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);// "Padding should not affect the final layoutSize in this circumstance";
	}
	{
		Widget widget{
			Widget::Args{
				.width = 100.f,
				.height = 100.f,
			},
			Widget::FlagsArgs::Default(),
		};

		Child container{Container{
			.widget{
				.width = 200.f,
				.height = 200.f,
			},
		}};

		widget.addChild(container);

		const vec2 maxSize{1000.f, 1000.f};

		vec2 expectedSize{100.f, 100.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);// "Widget should return its size when the size is fixed";

		widget.state.margin = 10.f;
		widget.state.padding = 10.f;
		expectedSize = {120.f, 120.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);// "Widget should return its size + margin when the size is fixed";
		widget.state.margin = 0.f;
		widget.state.padding = 0.f;

		widget.state.width = Size::Expand;
		widget.state.height = Size::Expand;
		expectedSize = maxSize;
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);// "Widget should return the maxSize when the size is expand";

		widget.state.margin = 10.f;
		widget.state.padding = 10.f;
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);
		widget.state.margin = 0.f;
		widget.state.padding = 0.f;

		widget.state.width = Size::Shrink;
		widget.state.height = Size::Shrink;
		expectedSize = {200.f, 200.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);

		widget.state.margin = 10.f;
		widget.state.padding = 10.f;
		expectedSize = {240.f, 240.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);
		widget.state.margin = 0.f;
		widget.state.padding = 0.f;

		widget.state.sizeConstraints.maxHeight = 50.f;
		widget.state.sizeConstraints.maxWidth = 50.f;
		expectedSize = {50.f, 50.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);
		widget.state.sizeConstraints.maxHeight = std::nullopt;
		widget.state.sizeConstraints.maxWidth = std::nullopt;

		widget.state.sizeConstraints.minHeight = 300.f;
		widget.state.sizeConstraints.minWidth = 300.f;
		expectedSize = {300.f, 300.f};
		REQUIRE(widget.layout(maxSize, {0, 0}, {}, true) == expectedSize);
	}
}