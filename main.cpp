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
			},
			.color = Color::RGBA(0.5, 0.5, 0.5, 1.0),
		}),
	}});
	window.run();
	return 0;
}
