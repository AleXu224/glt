#include "align.hpp"

using namespace squi;

Align::operator Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.xAlign = xAlign,
		.yAlign = yAlign,
	});
	auto &childFuncs = child->funcs();

	childFuncs.onArrange.emplace_back([storage](Widget &widget, vec2 &pos) {
		auto &data = widget.data();
		if (!data.parent) return;

		const auto maxOffset = data.parent->getContentSize() - widget.getLayoutSize();

		pos += (maxOffset * vec2(storage->xAlign, storage->yAlign)).rounded();
	});

	return child;
}