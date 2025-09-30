#include "core/app.hpp"

#include "include/widgets/column.hpp"
#include "include/widgets/scrollview.hpp"
#include "include/widgets/textBox.hpp"
#include "widgets/toggleButton.hpp"


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
		TextInput::Controller controller{"Hello"};
		bool disabled = false;

		bool toggled = false;

		Child build(const Element &) override {
			return Column{
				.children{
					TextBox{
						.disabled = disabled,
						.controller = controller,
						.validator = [](const std::string &text) -> std::optional<std::string> {
							std::println("Validating: {}", text);
							if (text.contains("error")) {
								return "Error: Text contains the word 'error'";
							}
							return {};
						},
					},
					Button{
						.onClick = [this]() {
							setState([&]() {
								// remove the last character if it exists
								if (!controller.getText().empty()) {
									controller.setText(controller.getText().substr(0, controller.getText().size() - 1));
								}
							});
						},
						.content = "Test",
					},
					Button{
						.onClick = [this]() {
							setState([&]() {
								disabled = !disabled;
							});
						},
						.content = "Toggle Disabled",
					},
					ToggleButton{
						.active = toggled,
						.onToggle = [this](bool active) {
							setState([&]() {
								toggled = active;
							});
						},
						.content = "Toggle",
					},
				},
			};
		}
	};
};

struct Test2 : StatefulWidget {
	// Args
	Key key;

	struct State : WidgetState<Test2> {
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
		.windowOptions{
			.width = 1280,
			.height = 720,
		},
		.child = Test{},
	};
	app.run();

	return 0;
}
