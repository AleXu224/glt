#include "widgets/dropdownButton.hpp"

#include "widgets/animatedText.hpp"
#include "widgets/fontIcon.hpp"
#include "widgets/navigator.hpp"
#include "widgets/row.hpp"
#include "widgets/transform.hpp"
#include "widgets/wrapper.hpp"

namespace squi {
	core::Child DropdownButton::State::build(const Element &element) {
		return Wrapper{
			.afterPosition = [this](RenderObject &renderObject) {
				contextMenuPosition = renderObject.getRect().getBottomLeft();
			},
			.child = Button{
				.onStatusChange = [&](Button::ButtonStatus status) {
					setState([&]() {
						this->status = status;
					});
				},
				.onClick = [&]() {
					setState([&]() {
						chevronRotation = 180.f;
					});
					Navigator::of(element).pushOverlay(Wrapper{
						.onUnmount = [&]() {
							setState([&]() {
								chevronRotation = 0.f;
							});
						},
						.child = ContextMenu{
							.position = contextMenuPosition,
							.items = widget->items,
						},
					});
				},
				.content = Row{
					.widget{
						.alignment = Alignment::CenterLeft,
					},
					.crossAxisAlignment = Flex::Alignment::center,
					.children{
						AnimatedText{
							.text = widget->text,
							.fontSize = widget->theme.fromStatus(status).textSize,
							.color = widget->theme.fromStatus(status).textColor,
						},
						Transform{
							.rotate = chevronRotation,
							.child = FontIcon{
								.color = widget->theme.fromStatus(status).textColor,
								.icon = 0xe313,
							},
						},
					},
				},
			},
		};
	}
}// namespace squi