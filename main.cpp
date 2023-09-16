#include "box.hpp"
#include "navigationView.hpp"
#include "window.hpp"
#include <print>

int main(int, char **) {
	using namespace squi;
	squi::Window window;
	window.addChild(NavigationView{
		.pages{
			NavigationView::Page{
				.name = "Blue page",
				.content = Box{
					.color = Color::RGBA(0, 0, 1, 0.5),
				},
			},
			NavigationView::Page{
				.name = "Red page",
				.icon = 0xEC4A,
				.content = Box{
					.color = Color::RGBA(1, 0, 0, 0.5),
				},
			},
		},
	});
	window.run();
	return 0;
}
