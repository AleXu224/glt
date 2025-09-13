#include "core/app.hpp"

#include "widgets/box.hpp"
#include "widgets/flex.hpp"
#include "widgets/gestureDetector.hpp"
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
						.width = Size::Expand,
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
		bool swapped = false;
		float spacing = 5.f;

		Child build(const Element &) override {
			return Gesture{
				.onUpdate = [this](const Gesture::State &state) {
					if (state.getScroll().y != 0.0f) {
						setState([&]() {
							spacing += state.getScroll().y;
						});
					}
					if (state.isKeyPressedOrRepeat(GLFW_KEY_SPACE)) {
						setState([this]() {
							swapped = !swapped;
						});
					}
				},
				.child = Flex{
					.direction = swapped ? Axis::Vertical : Axis::Horizontal,
					.crossAxisAlignment = Flex::Alignment::start,
					.spacing = spacing,
					.children{
						Box{.widget{.width = 50.f, .height = 50.f}, .color = Color::lime},
						ColorChanger{},
						Box{.widget{.width = 50.f, .height = Size::Expand}, .color = Color::lime},
						Box{.widget{.width = 50.f, .height = 50.f}, .color = Color::lime},
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
