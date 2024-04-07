#include "box.hpp"
#include "column.hpp"
#include "expander.hpp"
#include "window.hpp"
#include <print>
#include "numberBox.hpp"


int main(int /*unused*/, char ** /*unused*/) {
	using namespace squi;
	squi::Window window{};
	Observable<bool> focus{};
	window.addChild(Column{
		.children{
			Expander{
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
					.color = 0xFF0000FF,
				},
			},
			NumberBox{
				.value = 10,
				.min = 5,
				.max = 15,
				.step = 2,
				.onChange = [](float val){
					std::println("{}", val);
				},
				.controller{
					.focus = focus,
				},
			},
		},
	});
	focus.notify(true);
	window.run();
	return 0;
}
