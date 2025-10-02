#include "fontIcon.hpp"

#include <utf8cpp/utf8/cpp17.h>

namespace squi {
	Child FontIcon::build(const Element &) {
		auto args = widget;
		args.width = args.width.value_or(size);
		args.height = args.width.value_or(size);
		args.alignment = args.alignment.value_or(Alignment::Center);

		return Text{
			.widget = args,
			.text = utf8::utf32to8(std::u32string{icon}),
			.fontSize = fontSize,
			.font = font,
			.color = color,
		};
	}
}// namespace squi