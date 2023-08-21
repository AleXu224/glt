#include "container.hpp"
#include "widget.hpp"
#include "gtest/gtest.h"

using namespace squi;

TEST(Widget, layout) {
    {
        Widget widget{
            Widget::Args{
                .width = 100.f,
                .height = 100.f,
            },
            Widget::Flags::Default(),
        };

        const vec2 maxSize{1000.f, 1000.f};

        vec2 expectedSize{100.f, 100.f};
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize) << "Widget should return its size when the size is fixed";

		const vec2 smallerMaxSize{50.f, 50.f};
        expectedSize = {50.f, 50.f};
		EXPECT_EQ(widget.layout(smallerMaxSize, {0, 0}), expectedSize) << "Widget should return the maxSize when the size is larger than the maxSize";

		widget.setWidth(Size::Expand);
        widget.setHeight(Size::Expand);
        expectedSize = maxSize;
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize) << "Widget should return the maxSize when the size is expand";

		widget.setMargin(10.f);
        widget.setPadding(10.f);
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize);

		widget.state.sizeConstraints.maxHeight = 50.f;
        widget.state.sizeConstraints.maxWidth = 50.f;
		widget.setMargin(0.f);
		widget.setPadding(0.f);
		expectedSize = {50.f, 50.f};
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize) << "Widget should still follow the constraints when the size is expand";

		widget.setMargin(10.f);
		widget.setPadding(10.f);
        expectedSize = {70.f, 70.f};
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize);
		widget.setMargin(0.f);
        widget.setPadding(0.f);

		widget.state.sizeConstraints.maxHeight = std::nullopt;
        widget.state.sizeConstraints.maxWidth = std::nullopt;
        widget.setWidth(Size::Shrink);
        widget.setHeight(Size::Shrink);
        expectedSize = {0.f, 0.f};
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize) << "Widget has no content, padding or margin so the size should be 0";

		widget.setMargin(10.f);
        widget.setPadding(10.f);
        expectedSize = {40.f, 40.f};
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize) << "Widget has a margin and padding so the size should be 2 * margin + 2 * padding";
		widget.setMargin(0.f);
		widget.setPadding(0.f);

		widget.state.sizeConstraints.minHeight = 50.f;
        widget.state.sizeConstraints.minWidth = 50.f;
        expectedSize = {50.f, 50.f};
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize) << "Widget should still follow the constraints when the size is shrink";

		widget.setMargin(10.f);
        expectedSize = {70.f, 70.f};
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize) << "Widget has a margin so the size should be 2 * margin + size";

		widget.setPadding(10.f);
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize) << "Padding should not affect the final layoutSize in this circumstance";
	}
    {
        Widget widget{
            Widget::Args{
                .width = 100.f,
                .height = 100.f,
            },
            Widget::Flags::Default(),
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
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize) << "Widget should return its size when the size is fixed";

		widget.setMargin(10.f);
        widget.setPadding(10.f);
        expectedSize = {120.f, 120.f};
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize) << "Widget should return its size + margin when the size is fixed";
		widget.setMargin(0.f);
        widget.setPadding(0.f);

        widget.setWidth(Size::Expand);
        widget.setHeight(Size::Expand);
        expectedSize = maxSize;
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize) << "Widget should return the maxSize when the size is expand";

		widget.setMargin(10.f);
        widget.setPadding(10.f);
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize);
		widget.setMargin(0.f);
        widget.setPadding(0.f);

        widget.setWidth(Size::Shrink);
        widget.setHeight(Size::Shrink);
        expectedSize = {200.f, 200.f};
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize);

		widget.setMargin(10.f);
        widget.setPadding(10.f);
        expectedSize = {240.f, 240.f};
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize);
		widget.setMargin(0.f);
        widget.setPadding(0.f);

        widget.state.sizeConstraints.maxHeight = 50.f;
        widget.state.sizeConstraints.maxWidth = 50.f;
        expectedSize = {50.f, 50.f};
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize);
		widget.state.sizeConstraints.maxHeight = std::nullopt;
        widget.state.sizeConstraints.maxWidth = std::nullopt;

        widget.state.sizeConstraints.minHeight = 300.f;
        widget.state.sizeConstraints.minWidth = 300.f;
        expectedSize = {300.f, 300.f};
		EXPECT_EQ(widget.layout(maxSize, {0, 0}), expectedSize);
	}
}