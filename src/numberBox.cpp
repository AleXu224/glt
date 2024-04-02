#include "numberBox.hpp"
#include "box.hpp"
#include "gestureDetector.hpp"
#include "stack.hpp"
#include "textBox.hpp"
using namespace squi;


squi::NumberBox::operator squi::Child() const {
	auto storage = std::make_shared<Storage>();
	const auto &theme = TextBox::theme;

	return GestureDetector{
		.onEnter = [storage](auto) {
			storage->state.hovered = true;
			storage->stateObserver.notify(storage->state);
		},
		.onLeave = [storage](auto) {
			storage->state.hovered = false;
			storage->stateObserver.notify(storage->state);
		},
		.onUpdate = [storage](GestureDetector::Event event) {
			if (event.state.focused) storage->state.focused = true;
			if (event.state.focusedOutside) storage->state.focused = false;
			storage->stateObserver.notify(storage->state);
		},
		.child = Stack{
			.widget{widget.withDefaultHeight(32.f).withDefaultWidth(Size::Shrink).withSizeConstraints(SizeConstraints{.minWidth = 124.f})},
			.children{
				Box{
					.color{theme.rest},
					.borderColor{theme.border},
					.borderWidth{1.f},
					.borderRadius{4.f},
					.borderPosition = Box::BorderPosition::outset,
				},
				Box{
					.color{0.f},
					.borderColor{theme.bottomBorder},
					.borderWidth{0.f, 0.f, 1.f, 0.f},
					.borderRadius{4.f},
				},
			},
		},
	};
}
