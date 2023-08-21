#include "checkbox.hpp"
#include "align.hpp"
#include "box.hpp"
#include "fontIcon.hpp"
#include "gestureDetector.hpp"
#include "margin.hpp"
#include "row.hpp"
#include "text.hpp"


using namespace squi;

Checkbox::operator Child() const {
	auto storage = std::make_shared<Storage>(value);

	if (value.has_value()) storage->internalValue = value.value().get();

	return GestureDetector{
		.onClick = [storage, onChange = onChange](GestureDetector::Event event) {
			if (storage->value.has_value()) {
				bool &value = storage->value.value().get();
				value = !value;
				storage->internalValue = value;
				if (onChange) onChange(value);
			} else {
				storage->internalValue = !storage->internalValue;
				if (onChange) onChange(storage->internalValue);
			}
		},
		.child = Row{
			.widget = widget.withDefaultWidth(Size::Shrink).withDefaultHeight(Size::Shrink).withDefaultPadding(Padding{0.f, 8.f, 0.f, 4.f}),
			.alignment = Row::Alignment::center,
			.spacing = 8.f,
			.children{
				Box{
					.widget{
						.width = 20.f,
						.height = 20.f,
						.margin = Margin{0.f, 6.f},
						.onUpdate = [storage](Widget &w) {
							auto &box = dynamic_cast<Box::Impl &>(w);

							box.setColor(storage->internalValue ? Color::HEX(0x60CDFFFF) : Color::HEX(0x0));
							box.setBorderColor(storage->internalValue ? Color::HEX(0x60CDFFFF) : Color::RGBA(1, 1, 1, 0.6f));
						},
					},
					.color = Color::RGBA(0, 0, 0, 0.1f),
					.borderColor = Color::RGBA(1, 1, 1, 0.6f),
					.borderWidth = 1.f,
					.borderRadius = 4.f,
					.borderPosition = Box::BorderPosition::outset,
					.child = Align{
						.child = FontIcon{
							.widget{
								.onUpdate = [storage](Widget &w) {
									w.setVisible(storage->internalValue);
								},
							},
							.icon = 0xF78C,
							.size = 12.f,
							.color = Color::RGBA(0, 0, 0),
						},
					},
				},
				Text{
					.text = text.value_or("No text :("),
					.fontSize = 14.f,
					.lineWrap = true,
				},
			},
		},
	};
}