#include "widgets/toggleSwitch.hpp"
#include "window.hpp"


int main(int /*unused*/, char ** /*unused*/) {
	using namespace squi;

	VoidObservable sizeChange{};

	static size_t offset = 0;

	squi::Window window{};
	window.addChild(ToggleSwitch{});

	Window::run();
	return 0;
}
