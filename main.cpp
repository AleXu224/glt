#include "core/app.hpp"

#include "include/widgets/textInput.hpp"

#include <GLFW/glfw3.h>

using namespace squi::core;
using namespace squi;

// struct ColorChanger : StatefulWidget {
// 	// Args
// 	Key key;
// 	float size = 50.f;

// 	struct State : WidgetState<ColorChanger> {
// 		Color color = Color::royalblue;
// 		float width = 50.f;
// 		bool expanded = false;

// 		Child build(const Element &) override {
// 			return Gesture{
// 				.onClick = [this](const auto &) {
// 					setState([this]() {
// 						expanded = !expanded;
// 						color = expanded ? Color::black : Color::royalblue;
// 						width = expanded ? 100.f : 50.f;
// 					});
// 				},
// 				.child = AnimatedBox{
// 					.widget{
// 						.width = width,
// 						.height = widget->size,
// 					},
// 					.duration = 500ms,
// 					.color = color,
// 				},
// 			};
// 		}
// 	};
// };

struct Test : StatefulWidget {
	// Args
	Key key;

	struct State : WidgetState<Test> {
		Child build(const Element &) override {
			return TextInput{
				.active = true,
			};
		}
	};
};

int main(int /*unused*/, char ** /*unused*/) {

	App app{
		.windowOptions{
			.width = 1280,
			.height = 720,
		},
		.child = Test{},
	};
	app.run();

	return 0;
}
