#include "column.hpp"
#include "text.hpp"
#include "widgets/paginator.hpp"
#include "window.hpp"


int main(int /*unused*/, char ** /*unused*/) {
	using namespace squi;

	std::vector<int> items{};

	squi::Window window{};
	window.addChild(Paginator{
		.itemsPerPage = 2,
		.getItemCount = [=]() {
			return items.size();
		},
		.builder = [=](uint32_t offset, uint32_t count) {
			Children ret;
			std::span vals{items.begin() + offset, items.begin() + offset + count};
			for (const auto &val: vals) {
				ret.emplace_back(Text{.text = std::format("{}", val)});
			}
			return Column{
				.children = ret,
			};
		},
	});

	Window::run();
	return 0;
}
