#include "core/app.hpp"

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
		std::string text = "Hello, World!";
		float fontSize = 30.f;
		Color color = Color::white;

		Child build(const Element &) override {
			return Gesture{
				.onClick = [&](const Gesture::State &) {
					// Update the color to a random one
					setState([this]() {
						color = Color::css(rand() % 255, rand() % 255, rand() % 255);
					});
				},
				.onUpdate = [this](const Gesture::State &state) {
					if (state.getScroll().y != 0) {
						setState([this, &state]() {
							fontSize = std::clamp(fontSize + state.getScroll().y, 5.f, 100.f);
						});
					}
					if (!state.textInput.empty()) {
						setState([this, &state]() {
							text += state.textInput;
						});
					}
					if (state.isKeyPressedOrRepeat(GLFW_KEY_BACKSPACE) && !text.empty()) {
						setState([this]() {
							text.pop_back();
						});
					}
				},
				.child = Text{
					.text = text,
					.fontSize = fontSize,
					.color = color,
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
