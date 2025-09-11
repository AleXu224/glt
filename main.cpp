#include "core/app.hpp"

#include "widgets/box.hpp"
#include "widgets/gestureDetector.hpp"
using namespace squi::core;
using namespace squi;

struct ColorChanger : StatefulWidget {
	// Args
	Key key;
	Color color = Color::royalblue;

	struct State : WidgetState<ColorChanger> {
		Color color = Color::black;
		bool expanded = false;

		Child build(const Element &element) override {
			return GestureDetector{
				.onClick = [this](auto) {
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

struct Dragger : StatefulWidget {
	// Args
	Key key;

	struct State : WidgetState<Dragger> {
		float width = 100.f;
		float height = 100.f;

		Child build(const Element &element) override {
			return GestureDetector{
				.onDrag = [this](GestureDetector::State state) {
					auto delta = state.getDragDelta();
					setState([&]() {
						width += delta.x;
						height += delta.y;
					});
				},
				.child = Box{
					.widget{
						.width = width,
						.height = height,
					},
					.child = ColorChanger{
						.color = Color::silver,
					},
				},
			};
		}
	};
};

struct Test : StatelessWidget {
	// Args
	Key key;

	Child build(const Element &) const {
		return Dragger{};
	}
};

int main(int /*unused*/, char ** /*unused*/) {

	App app{
		.child = Test{},
	};
	app.run();

	return 0;
}
