#include "fontIcon.hpp"
#include "align.hpp"
#include "container.hpp"
#include "text.hpp"
#include <utf8/cpp20.h>


using namespace squi;

squi::FontIcon::operator Child() const {
	return Container{
		.widget = widget.withDefaultWidth(size)
					  .withDefaultHeight(size),
		.bounded = false,
		.child{
			Align{
				.child{
					Text{
						.widget = textWidget,
						.text{utf8::utf32to8(std::u32string{icon})},
						.fontSize = 24.f,
						.font{font},
						.color{color},
					},
				},
			},
		},
	};
}
