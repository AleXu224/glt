#include "core/app.hpp"

#include "widgets/box.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/stack.hpp"
#include <GLFW/glfw3.h>

using namespace squi::core;
using namespace squi;

struct ColorChanger : StatefulWidget {
	// Args
	Key key;
	Color color = Color::royalblue;
	float size = 50.f;

	struct State : WidgetState<ColorChanger> {
		Color color = Color::black;
		bool expanded = false;

		Child build(const Element &) override {
			return Gesture{
				.widget{
					.alignment = Alignment::Center,
				},
				.onClick = [this](const auto &) {
					setState([this]() {
						color = (color == Color::black) ? widget->color : Color::black;
						expanded = !expanded;
					});
				},
				.child = Box{
					.widget{
						.width = widget->size,
						.height = widget->size,
						.margin = Margin{10.f},
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
		bool swapped = false;

		Child build(const Element &) override {
			return Gesture{
				.onUpdate = [this](const Gesture::State &state) {
					if (state.isKeyPressedOrRepeat(GLFW_KEY_SPACE)) {
						setState([this]() {
							swapped = !swapped;
						});
					}
				},
				.child = Box{
					.widget{
						.width = 300.f,
						.height = 300.f,
					},
					.child = Stack{
						.children{
							ColorChanger{
								.key = ValueKey("red"),
								.color = Color::red,
								.size = 150.f,
							},
							swapped//
								? Child(ColorChanger{
									  .key = ValueKey("orangeRed"),
									  .color = Color::orangered,
									  .size = 100.f,
								  })
								: nullptr,
							ColorChanger{
								.key = ValueKey("orange"),
								.color = Color::orange,
								.size = 50.f,
							},
						},
					},
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
