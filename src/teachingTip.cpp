#include "teachingTip.hpp"

#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "column.hpp"
#include "fontIcon.hpp"
#include "observer.hpp"
#include "row.hpp"
#include "stack.hpp"
#include "text.hpp"

squi::TeachingTip::operator squi::Child() const {
	VoidObservable closeEvent{};

	Child content = Row{
		.children{
			Column{
				.widget{
					.padding = Padding{12.f, 0.f, 12.f, 9.f},
				},
				.children{
					title.empty() ? Child{} : Text{
												  .text = title,
												  .font = FontStore::defaultFontBold,
											  },
					Text{
						.text = message,
					},
				},
			},
			Button{
				.widget{
					.width = 30.f,
					.height = 30.f,
					.margin = 5.f,
					.padding = 0.f,
				},
				.style = ButtonStyle::Subtle(),
				.onClick = [closeEvent](GestureDetector::Event) {
					closeEvent.notify();
				},
				.child = Align{.child = FontIcon{.icon = 0xe5cd, .size = 10.f}},
			},
		},
	};

	return Stack{
		.widget{
			.onInit = [closeEvent](Widget &w) {
				w.customState.add(closeEvent.observe([&w]() {
					w.deleteLater();
				}));
			},
		},
		.children{
			Box{
				.widget{
					.width = Size::Shrink,
					.height = Size::Shrink,
					.onUpdate = [target = target, closeEvent](Widget &) {
						if (target.expired()) closeEvent.notify();
					},
					.onArrange = [targetRef = target](Widget &w, vec2 &pos) {
						if (auto target = targetRef.lock()) {
							pos = target->getPos()
									  .withXOffset(target->getSize().x / 2.f - w.getSize().x / 2.f)
									  .withYOffset(-w.getSize().y);
						}
					},
				},
				.color = 0x282828FF,
				.borderColor{0.46f, 0.46f, 0.46f, 0.4f},
				.borderWidth{1.f},
				.borderRadius{8.f},
				.child = content,
			},
		},
	};
}
