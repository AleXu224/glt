#include "core/animated.hpp"
#include "core/app.hpp"

#include "include/widgets/animatedBox.hpp"
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
		Color color = Color::royalblue;
		float width = 50.f;
		bool expanded = false;

		Child build(const Element &) override {
			return Gesture{
				.onClick = [this](const auto &) {
					setState([this]() {
						expanded = !expanded;
						color = expanded ? Color::black : Color::royalblue;
						width = expanded ? 100.f : 50.f;
					});
				},
				.child = AnimatedBox{
					.widget{
						.width = width,
						.height = widget->size,
					},
					.duration = 500ms,
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
					Button{
						.onClick = [this]() {
							setState([this]() {
								toggle = !toggle;
							});
						},
						.content = "Toggle",
					},
					Button{
						.theme = Button::Theme::Accent(),
						.disabled = toggle,
						.content = "Button",
					},
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
