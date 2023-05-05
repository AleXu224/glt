#define NOMINMAX
#include "fontIcon.hpp"
#include "align.hpp"
#include "container.hpp"
#include "text.hpp"
#include "vec2.hpp"
#include <limits>

using namespace squi;

squi::FontIcon::operator Child() const {
	return Container{
		.widget{
			.width = size,
			.height = size,
			.margin = margin,
            .onArrange = [](auto&, auto){
                1;
            },
		},
		.child{
			Align{
				.child{
					Text{
						.text{icon},
						.fontSize = size,
						.fontPath{font},
						.color{color},
					},
				},
			},
		},
	};
}
