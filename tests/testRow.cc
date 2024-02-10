#include "container.hpp"
#include "row.hpp"
#include "gtest/gtest.h"

using namespace squi;

// A Widget whose height depends on its width
struct TextLikeWidget {
	// Args
	Widget::Args widget{};

	class Impl : public Widget {
		// Data

	public:
		Impl(const TextLikeWidget &args) : Widget(args.widget, Widget::Flags::Default()) {}

		vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink) final {
			if (shouldShrink.width) {
				return minSize.withY(200.f);
			}
			return minSize.withY(200.f - maxSize.x);
		}
	};

	operator Child() const {
		return std::make_shared<Impl>(*this);
	}
};

TEST(Row, layout) {
	Child row1 = Row{
		.widget{
			.width = 200.f,
			.height = 200.f,
		},
	};
	Child row2 = Row{
		.widget{
			.width = Size::Shrink,
			.height = 200.f,
		},
	};
	Child row3 = Row{
		.widget{
			.width = 200.f,
			.height = Size::Shrink,
		},
	};
	Child row4 = Row{
		.widget{
			.width = Size::Shrink,
			.height = Size::Shrink,
		},
	};
	Child row5 = Row{
		.widget{
			.width = Size::Shrink,
			.height = Size::Shrink,
		},
		.spacing = 10.f,
	};

	auto rows = {row1, row2, row3, row4, row5};

	const vec2 maxSize = {1000.f, 1000.f};

	EXPECT_EQ(row1->layout(maxSize, {0, 0}), (vec2{200, 200}));
	EXPECT_EQ(row2->layout(maxSize, {0, 0}), (vec2{0, 200}));
	EXPECT_EQ(row3->layout(maxSize, {0, 0}), (vec2{200, 0}));
	EXPECT_EQ(row4->layout(maxSize, {0, 0}), (vec2{0, 0}));
	EXPECT_EQ(row5->layout(maxSize, {0, 0}), (vec2{0, 0}));

	// Fixed sized children
	Children children1{
		Container{
			.widget{
				.width = 50.f,
				.height = 50.f,
			},
		},
		Container{
			.widget{
				.width = 50.f,
				.height = 50.f,
			},
		},
	};
	for (auto &row: rows) row->setChildren(children1);

	EXPECT_EQ(row1->layout(maxSize, {0, 0}), (vec2{200, 200}));
	EXPECT_EQ(row2->layout(maxSize, {0, 0}), (vec2{100, 200}));
	EXPECT_EQ(row3->layout(maxSize, {0, 0}), (vec2{200, 50}));
	EXPECT_EQ(row4->layout(maxSize, {0, 0}), (vec2{100, 50}));
	EXPECT_EQ(row5->layout(maxSize, {0, 0}), (vec2{110, 50}));

	Children children2{
		Container{
			.widget{
				.width = 50.f,
				.height = 50.f,
			},
		},
		Container{
			.widget{
				.width = Size::Expand,
				.height = 50.f,
				.sizeConstraints{
					.minWidth = 50.f,
				},
			},
		},
		Container{
			.widget{
				.width = Size::Expand,
				.height = 50.f,
				.sizeConstraints{
					.minWidth = 50.f,
				},
			},
		},
	};
	for (auto &row: rows) row->setChildren(children2);

	EXPECT_EQ(row1->layout(maxSize, {0, 0}), (vec2{200, 200}));
	EXPECT_EQ(row2->layout(maxSize, {0, 0}), (vec2{150, 200}));
	EXPECT_EQ(row3->layout(maxSize, {0, 0}), (vec2{200, 50}));
	EXPECT_EQ(row4->layout(maxSize, {0, 0}), (vec2{150, 50}));
	EXPECT_EQ(row5->layout(maxSize, {0, 0}), (vec2{170, 50}));

	Children children3 {
		Container{
			.widget{
				.width = 100.f,
				.height = 50.f,
			},
		},
		TextLikeWidget{
			.widget{
				.width = Size::Expand,
				.height = Size::Shrink,
			},
		},
	};
	for (auto &row: rows) row->setChildren(children3);

	const auto maxSize2 = vec2{150.f, 1000.f};

	EXPECT_EQ(row1->layout(maxSize2, {0, 0}), (vec2{150, 200}));
	EXPECT_EQ(row2->layout(maxSize2, {0, 0}), (vec2{100, 200}));
	EXPECT_EQ(row3->layout(maxSize2, {0, 0}), (vec2{150, 150}));
	EXPECT_EQ(row4->layout(maxSize2, {0, 0}), (vec2{100, 200}));
	EXPECT_EQ(row5->layout(maxSize2, {0, 0}), (vec2{110, 200}));
}