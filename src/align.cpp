#include "align.hpp"

#include "widget.hpp"

using namespace squi;

Align::operator Child() {
	if (child) {
		auto storage = std::make_shared<Storage>(Storage{
			.xAlign = xAlign,
			.yAlign = yAlign,
		});
		auto &childFuncs = child->funcs();

		childFuncs.onArrange.emplace_back([storage](Widget &widget, vec2 &pos) {
			if (!*widget.state.parent) return;

			const auto maxOffset = widget.state.parent->getContentSize() - widget.getLayoutSize();

			pos += (maxOffset * vec2(storage->xAlign, storage->yAlign)).rounded();
		});
	}

	return child;
}