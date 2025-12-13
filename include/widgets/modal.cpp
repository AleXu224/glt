#include "modal.hpp"
#include "navigator.hpp"
#include "widgets/box.hpp"
#include "widgets/gestureDetector.hpp"
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
						.color{0.f, 0.f, 0.f, 0.4f},
					},
				},
				widget->child,
			},
		};
	}

	void Modal::State::observeCloseEvent() {
		closeObserver = widget->closeEvent.observe([this]() {
			Navigator::of(*this->element).popOverlay();
		});
	}

	void Modal::State::initState() {
		observeCloseEvent();
	}

	void Modal::State::widgetUpdated() {
		observeCloseEvent();
	}
}// namespace squi