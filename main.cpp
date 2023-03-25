#include "window.hpp"
#include "child.hpp"
#include "widget.hpp"
#include "color.hpp"
#include "box.hpp"

int main(int, char **) {
	using namespace squi;
	squi::Window window;
	window.addChild(new Box{Box::Args{
		.widget{
			.size = {200, 200},
			.margin = {10},
			.padding = {10},
		},
		.borderRadius = 10,
		.child = new Box(Box::Args{
			.widget {
				.size = {50, 50},
				.margin{10},
				.onInit = [] (Widget &widget) {
					auto &data = widget.data();
					data.size.x.animateTo(100, 5s);
					data.size.y.animateTo(200, 2.5s);
					data.margin.left.animateTo(40, 1s);
				},
			},
			.color = Color::RGBA(0.5, 0.5, 0.5, 1.0),
			.borderWidth = 1,
			.borderRadius = 100,
		}),
	}});
	window.run();
	return 0;
}
