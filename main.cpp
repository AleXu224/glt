#include "box.hpp"
#include "child.hpp"
#include "color.hpp"
#include "widget.hpp"
#include "window.hpp"

int main(int, char **) {
	using namespace squi;
	squi::Window window;
	window.addChild(new Box{Box::Args{
		.widget{
			.size = {200, 200},
			.margin = {10},
			.padding = {10},
		},
		.color = Color::RGBA(0.0, 0.5, 0.5, 1.0),
		.borderRadius = 10,
		.child = new Box(Box::Args{
			.widget{
				//				.size = {50, 50},
				//				.margin{10},
				.size = {100, 200},
				.margin = {10, 10, 10, 40},
				.onInit = [](Widget &widget) {
					auto &data = widget.data();
					//					data.size.x.animateTo(100, 5s);
					//					data.size.y.animateTo(200, 2.5s);
					//					data.margin.left.animateTo(40, 1s);
					data.gestureDetector.onClick = [&data](GestureDetector &gd) {
					  	static bool isBig = false;
					  	if (isBig) {
							data.size.x.animateTo(100, 200ms);
							isBig = false;
						} else {
							data.size.x.animateTo(120, 200ms);
							isBig = true;
						}
					};
					data.gestureDetector.onEnter = [&widget](GestureDetector &gd) {
						((Box&)widget).setColor(Color::RGBA(0.0, 0.5, 0.0, 1.0));
					};
					data.gestureDetector.onLeave = [&widget](GestureDetector &gd) {
						((Box&)widget).setColor(Color::RGBA(0.5, 0.5, 0.5, 1.0));
					};
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
