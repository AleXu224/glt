#include "navigationMenu.hpp"
#include "button.hpp"
#include "column.hpp"
#include "row.hpp"
#include "text.hpp"
#include "vec2.hpp"

using namespace squi;

NavigationMenu::operator Child() const {
    auto storage = std::make_shared<Storage>();

	return Column{
		.widget{
			.width = 320.f,
			.height = Size::Expand,
		},
		.children{
			Row{
                .widget{
                    .height = 40.f,
                },
                .alignment = Row::Alignment::center,
                .spacing = 10.f,
				.children{
					Button{
						.text = "Home",
						.style = ButtonStyle::Subtle(),
					},
                    Text{
                        .text{"Header"},
                    },
				},
			},
		},
	};
}