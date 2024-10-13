#include "modal.hpp"

#include "align.hpp"
#include "box.hpp"
#include "gestureDetector.hpp"
#include "registerEvent.hpp"
#include "stack.hpp"
#include <GLFW/glfw3.h>

using namespace squi;

squi::Modal::operator squi::Child() const {
	return RegisterEvent{
		.onInit = [closeEvent = closeEvent](Widget &w) {
			observe(w, closeEvent, [&w]() {
				w.deleteLater();
			});
		},
		.child = Stack{
			.widget = widget,
			.children{
				GestureDetector{
					.onClick = [closeEvent = closeEvent](GestureDetector::Event) {
						closeEvent.notify();
					},
					.onUpdate = [closeEvent = closeEvent](GestureDetector::Event event) {
						if (event.state.isKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) {
							closeEvent.notify();
						}
					},
					.child = Box{
						.color{0.f, 0.f, 0.f, 0.4f},
					},
				},
				Align{
					.child = child,
				},
			},
		},
	};
}
