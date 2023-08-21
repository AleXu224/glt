#include <utf8/cpp17.h>
#define NOMINMAX
#include "align.hpp"
#include "container.hpp"
#include "fontIcon.hpp"
#include "text.hpp"


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
