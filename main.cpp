#include "core/app.hpp"

#include "widgets/box.hpp"

int main(int /*unused*/, char ** /*unused*/) {
	using namespace squi::core;
	using namespace squi;

	App app{
		.child = Box{
			.widget{
				.width = 100.f,
				.height = 100.f,
			},
		}
	};
	app.run();

	return 0;
}
