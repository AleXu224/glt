#include "fontIcon.hpp"
#include "align.hpp"
#include "container.hpp"
#include "text.hpp"
#include <utf8/cpp17.h>



using namespace squi;

squi::FontIcon::operator Child() const {
	return Container{
		.widget = widget
					  .withDefaultWidth(size)
					  .withDefaultHeight(size),
		.child{
			Align{
				.child{
					Text{
						.widget = textWidget,
						.text{utf8::utf32to8(std::u32string{icon})},
						.fontSize = size,
						.font{font},
						.color{color},
					},
				},
			},
		},
	};
}
