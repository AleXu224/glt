#include "window.hpp"
#include "child.hpp"
#include "widget.hpp"
#include "color.hpp"
#include "box.hpp"

int main(int, char **) {
	using namespace squi;
	Color c = Color::RGBA(0.5, 0.5, 0.5, 1.0);
	printf("%x\n", c.value);
	Child a = new Widget(Widget::Args{
		.size = {150, 100},
	}, Widget::Options{});
	Child b = new Box(Box::Args{});
	printf("%f\n", a->getSize().x);
	squi::Window window;
	window.addChild(new Box{Box::Args{
		.widget{
			.size = {200, 200},
		}
	}});
	window.run();
	return 0;
}
