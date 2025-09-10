#include "core/app.hpp"

#include "widgets/box.hpp"
#include "widgets/gestureDetector.hpp"
using namespace squi::core;
using namespace squi;

struct ColorChanger : StatefulWidget {
	// Args
	Color color = Color::royalblue;

	struct State : WidgetState<ColorChanger> {
		Color color = Color::black;
		bool expanded = false;

		Child build(const Element &element) override {
			return GestureDetector{
				.onClick = [this](auto) {
					std::println("GestureDetector clicked!");
					setState([this]() {
						color = (color == Color::black) ? widget->color : Color::black;
						expanded = !expanded;
					});
				},
				.child = Box{
					.widget{
						.width = expanded ? 80.f : 50.f,
						.height = 50.f,
						.margin = Margin{10.f},
					},
					.color = color,
				},
			};
		}
	};
};

struct Test : StatelessWidget {
	// Args

	Child build(const Element &) const {
		return Box{
			.widget{
				.width = 100.f,
				.height = 100.f,
			},
			.child = ColorChanger{},
		};
	}
};

int main(int /*unused*/, char ** /*unused*/) {

	App app{
		.child = Test{},
	};
	app.run();

	return 0;
}
