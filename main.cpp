#include "core/app.hpp"

#include "widgets/box.hpp"
using namespace squi::core;
using namespace squi;

struct Test : StatelessWidget {
	// Args

	Child build(const Element &) const {
		return Box{
			.widget{
				.width = 100.f,
				.height = 100.f,
			},
			.child = Box{
				.widget{
					.width = 50.f,
					.height = 50.f,
					.margin = Margin{10.f},
				},
				.color = Color::red,
			},
		};
	}
};

int main(int /*unused*/, char ** /*unused*/) {

	App app{
		.child = Test{},
	};
	app.run();

	return 0;
}
