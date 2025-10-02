#include "fontIcon.hpp"

#include <utf8cpp/utf8/cpp17.h>

namespace squi {
	Child FontIcon::build(const Element &) {
		return Text{
			.widget{
				.width = size,
				.height = size,
				.alignment = Alignment::Center,
				.sizeConstraints = BoxConstraints{
					.maxWidth = size,
					.maxHeight = size,
				},
			},
			.text = utf8::utf32to8(std::u32string{icon}),
			.fontSize = fontSize,
			.font = font,
			.color = color,
		};
	}
}// namespace squi