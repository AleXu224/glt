#include "align.hpp"

using namespace squi;

Align::operator Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.xAlign = xAlign,
		.yAlign = yAlign,
	});
	auto &childFuncs = child->funcs();

	childFuncs.onDraw = [storage, oldOnDraw = childFuncs.onDraw](Widget &widget) {
		if (oldOnDraw) oldOnDraw(widget);
		auto &data = widget.data();
		if (!data.parent) return;

		const auto maxOffset = data.parent->getContentRect().size() - widget.getLayoutRect().size();

		data.pos = data.pos + maxOffset * vec2(storage->xAlign, storage->yAlign);
	};

	return child;
}