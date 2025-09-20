#include "core/animated.hpp"
#include "core/app.hpp"

#include "include/widgets/button.hpp"
#include "include/widgets/scrollview.hpp"
#include "widgets/box.hpp"
#include "widgets/flex.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/text.hpp"
#include <GLFW/glfw3.h>

using namespace squi::core;
using namespace squi;

struct ColorChanger : StatefulWidget {
	// Args
	Key key;
	float size = 50.f;

	struct State : WidgetState<ColorChanger> {
		Animated<Color> color{
			.from = Color::royalblue,
			.duration = 500ms,
			.curve = Curve::easeOutCubic,
		};
		Animated<float> width{
			.from = 50.f,
			.duration = 500ms,
			.curve = Curve::easeOutCubic,
		};
		bool expanded = false;

		void initState() override {
			color.mount(this);
			width.mount(this);
		}

		Child build(const Element &) override {
			return Gesture{
				.onClick = [this](const auto &) {
					setState([this]() {
						expanded = !expanded;
						color = expanded ? Color::black : Color::royalblue;
						width = expanded ? 100.f : 50.f;
					});
				},
				.child = Box{
					.widget{
						.width = width,
						.height = widget->size,
					},
					.color = color,
				},
			};
		}
	};
};

struct Test : StatefulWidget {
	// Args
	Key key;

	struct State : WidgetState<Test> {
		bool toggle = false;
		Button::Status status = Button::Status::resting;

		Child build(const Element &) override {
			return ScrollView{
				.direction = Axis::Horizontal,
				.spacing = 2.f,
				.children{
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
				},
			};
		}
	};
};

int main(int /*unused*/, char ** /*unused*/) {

	App app{
		.child = Test{},
	};
	app.run();

	return 0;
}
