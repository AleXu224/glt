#include "core/app.hpp"

#include "include/widgets/button.hpp"
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
		bool toggle = false;
		Button::Status status = Button::Status::resting;

		Child build(const Element &) override {
			return Button{
				.widget{
					.margin = 16.f,
				},
				.theme = toggle ? Button::Theme::Accent() : Button::Theme::Standard(),
				.onStatusChange = [this](Button::Status status) {
					setState([this, status]() {
						this->status = status;
					});
				},
				.onClick = [this]() {
					setState([this]() {
						toggle = !toggle;
					});
				},
				.content = Box{
					.widget{
						.width = 16.f,
						.height = 16.f,
						.alignment = Alignment::Center,
					},
					.color = Button::Theme::Standard().fromStatus(status).textColor,
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
