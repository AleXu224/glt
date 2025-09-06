#include "core/app.hpp"

int main(int /*unused*/, char ** /*unused*/) {
	using namespace squi::core;

	App app{};
	app.run();

	return 0;
}
