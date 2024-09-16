#include "column.hpp"
#include "dropdownButton.hpp"
#include "expander.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <print>


int main(int /*unused*/, char ** /*unused*/) {
	using namespace squi;

	VoidObservable sizeChange{};

	static size_t offset = 0;

	squi::Window window{};
	window.addChild(Column{
		.children{
			Align{
				.child = DropdownButton{
					.style = ButtonStyle::Standard(),
					.text = "Whoah second window",
					.items{
						ContextMenu::Item{
							.text = "asd",
						},
					},
				},
			},
		},
	});

	squi::Window window2{};
	window2.addChild(Column{
		.children{
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
	Window::run();
	return 0;
}
