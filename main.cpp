#include "box.hpp"
#include "expander.hpp"
#include "window.hpp"
#include <print>

int main(int, char **) {
	using namespace squi;
	squi::Window window;
	window.addChild(Expander{
		.widget{
			.margin = 8.f,
		},
		.icon = 0xE82F,
		.heading = "This is a heading",
		.caption = "This is a caption",
		.child = Box{
			.widget{
				.width = 16.f,
				.height = 16.f,
			},
			.color = Color::HEX(0xFF0000FF),
		},
	});
	window.run();
	return 0;
}
