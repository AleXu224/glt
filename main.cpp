#include "box.hpp"
#include "column.hpp"
#include "dropdownButton.hpp"
#include "expander.hpp"
#include "gestureDetector.hpp"
#include "text.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <print>


int main(int /*unused*/, char ** /*unused*/) {
	using namespace squi;

	std::string_view str = "(ATK 1514.54 * 258.65% + Passive 2 : (Elemental Mastery 313.50 * 150.00%)) * (100.00% + Electro DMG% 46.60%) * (100.00% + Crit Rate 97.30% * Crit DMG 205.00%) * 48.70% * 90.00%";

	VoidObservable sizeChange{};

	static size_t offset = 0;

	squi::Window window{};
	window.addChild(Column{
		.children{
			// Expander{
			// 	.widget{
			// 		.margin = 8.f,
			// 	},
			// 	.icon = 0xE82F,
			// 	.heading = "This is a heading",
			// 	.caption = "This is a caption",
			// 	.child = Box{
			// 		.widget{
			// 			.width = 16.f,
			// 			.height = 16.f,
			// 		},
			// 		.color = 0xFF0000FF,
			// 	},
			// },
			Align{
				.child = DropdownButton{
					.style = ButtonStyle::Standard(),
					.text = "some text",
					.items{
						ContextMenu::Item{
							.text = "asd",
						},
					},
				},
			},
		},
	});
	window.run();
	return 0;
}
