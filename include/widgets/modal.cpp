#include "modal.hpp"
#include "navigator.hpp"
#include "widgets/box.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/slideIn.hpp"
#include "widgets/stack.hpp"


namespace squi {
	core::Child Modal::State::build(const Element &) {
		return Stack{
			.children{
				Gesture{
					.onClick = [this](auto &&) {
						widget->closeEvent.notify();
					},
					.onUpdate = [this](const Gesture::State &event) {
						if (event.isKeyPressedOrRepeat(GestureKey::escape)) {
							widget->closeEvent.notify();
						}
					},
					.child = Box{
						.color = backgroundColor,
					},
				},
				SlideIn{
					.direction = Direction::bottom,
					.followChild = true,
					.child = widget->child,
				},
			},
		};
	}

	void Modal::State::observeCloseEvent() {
		closeObserver = widget->closeEvent.observe([this]() {
			if (Navigator::of(*this->element).is(*this->element))
				Navigator::of(*this->element).popOverlay();
		});
	}

	void Modal::State::initState() {
		observeCloseEvent();

		backgroundColor.mount(this);
		backgroundColor = Color{0.f, 0.f, 0.f, 0.4f};
	}

	void Modal::State::widgetUpdated() {
		observeCloseEvent();
	}
}// namespace squi