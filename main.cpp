#include "align.hpp"
#include "window.hpp"

#include "widgets/slider.hpp"

int main(int /*unused*/, char ** /*unused*/) {
	using namespace squi;

	squi::Window window{};
	window.addChild(Align{
		.child = Slider{
			.minVal = 1.f,
			.maxVal = 10.f,
			.step = 1.f,
			.valueChanged = [](float newVal) {
				std::println("{}", newVal);
			},
		},
	});

	Window::run();
	return 0;
}
