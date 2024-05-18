#include "column.hpp"
#include "container.hpp"
#include "gtest/gtest.h"


using namespace squi;

// A Widget whose width depends on its height
struct InvTextLikeWidget {
	// Args
	Widget::Args widget{};

	class Impl : public Widget {
		// Data

	public:
		Impl(const InvTextLikeWidget &args) : Widget(args.widget, Widget::FlagsArgs::Default()) {}

		vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool /*final*/) final {
			if (shouldShrink.height) {
				return minSize.withX(200.f);
			}
			return minSize.withX(200.f - maxSize.y);
		}
	};

	operator Child() const {
		return std::make_shared<Impl>(*this);
	}
};

TEST(Column, layout) {
	Child column1 = Column{
		.widget{
			.width = 200.f,
			.height = 200.f,
		},
	};
	Child column2 = Column{
		.widget{
			.width = 200.f,
			.height = Size::Shrink,
		},
	};
	Child column3 = Column{
		.widget{
			.width = Size::Shrink,
			.height = 200.f,
		},
	};
	Child column4 = Column{
		.widget{
			.width = Size::Shrink,
			.height = Size::Shrink,
		},
	};
	Child column5 = Column{
		.widget{
			.width = Size::Shrink,
			.height = Size::Shrink,
		},
		.spacing = 10.f,
	};

	auto columns = {column1, column2, column3, column4, column5};

	const vec2 maxSize = {1000.f, 1000.f};

	EXPECT_EQ(column1->layout(maxSize, {0, 0}, {}, true), (vec2{200, 200}));
	EXPECT_EQ(column2->layout(maxSize, {0, 0}, {}, true), (vec2{200, 0}));
	EXPECT_EQ(column3->layout(maxSize, {0, 0}, {}, true), (vec2{0, 200}));
	EXPECT_EQ(column4->layout(maxSize, {0, 0}, {}, true), (vec2{0, 0}));
	EXPECT_EQ(column5->layout(maxSize, {0, 0}, {}, true), (vec2{0, 0}));

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
	for (const auto &column: columns) column->setChildren(children1);

	EXPECT_EQ(column1->layout(maxSize, {0, 0}, {}, true), (vec2{200, 200}));
	EXPECT_EQ(column2->layout(maxSize, {0, 0}, {}, true), (vec2{200, 100}));
	EXPECT_EQ(column3->layout(maxSize, {0, 0}, {}, true), (vec2{50, 200}));
	EXPECT_EQ(column4->layout(maxSize, {0, 0}, {}, true), (vec2{50, 100}));
	EXPECT_EQ(column5->layout(maxSize, {0, 0}, {}, true), (vec2{50, 110}));

	Children children2{
		Container{
			.widget{
				.width = 50.f,
				.height = 50.f,
			},
		},
		Container{
			.widget{
				.width = 50.f,
				.height = Size::Expand,
				.sizeConstraints{
					.minHeight = 50.f,
				},
			},
		},
		Container{
			.widget{
				.width = 50.f,
				.height = Size::Expand,
				.sizeConstraints{
					.minHeight = 50.f,
				},
			},
		},
	};
	for (const auto &column: columns) column->setChildren(children2);

	EXPECT_EQ(column1->layout(maxSize, {0, 0}, {}, true), (vec2{200, 200}));
	EXPECT_EQ(column2->layout(maxSize, {0, 0}, {}, true), (vec2{200, 150}));
	EXPECT_EQ(column3->layout(maxSize, {0, 0}, {}, true), (vec2{50, 200}));
	EXPECT_EQ(column4->layout(maxSize, {0, 0}, {}, true), (vec2{50, 150}));
	EXPECT_EQ(column5->layout(maxSize, {0, 0}, {}, true), (vec2{50, 170}));

	Children children3{
		Container{
			.widget{
				.width = 50.f,
				.height = 100.f,
			},
		},
		InvTextLikeWidget{
			.widget{
				.width = Size::Shrink,
				.height = Size::Expand,
			},
		},
	};
	for (const auto &column: columns) column->setChildren(children3);

	const auto maxSize2 = vec2{1000.f, 150.f};

	EXPECT_EQ(column1->layout(maxSize2, {0, 0}, {}, true), (vec2{200, 150}));
	EXPECT_EQ(column2->layout(maxSize2, {0, 0}, {}, true), (vec2{200, 100}));
	EXPECT_EQ(column3->layout(maxSize2, {0, 0}, {}, true), (vec2{150, 150}));
	EXPECT_EQ(column4->layout(maxSize2, {0, 0}, {}, true), (vec2{200, 100}));
	EXPECT_EQ(column5->layout(maxSize2, {0, 0}, {}, true), (vec2{200, 110}));
}