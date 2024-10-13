#include "window.hpp"

#include "widgets/liteFilter.hpp"

int main(int /*unused*/, char ** /*unused*/) {
	using namespace squi;

	VoidObservable sizeChange{};

	static size_t offset = 0;

	squi::Window window{};
	window.addChild(LiteFilter{
		.multiSelect = true,
		.items{
			LiteFilter::Item{
				.name = "Pyro",
				.onUpdate = [](bool active) {
					std::println("Pyro : {}", active);
				},
			},
			LiteFilter::Item{
				.name = "Electro",
				.onUpdate = [](bool active) {
					std::println("Electro : {}", active);
				},
			},
			LiteFilter::Item{
				.name = "Hydro",
				.onUpdate = [](bool active) {
					std::println("Hydro : {}", active);
				},
			},
		},
	});

	Window::run();
	return 0;
}
