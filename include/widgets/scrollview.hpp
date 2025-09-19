#pragma once

#include "core/core.hpp"
#include "widgets/scrollable.hpp"
#include "widgets/scrollbar.hpp"
#include "widgets/stack.hpp"


namespace squi {
	struct ScrollView : StatefulWidget {
		// Args
		Key key;
		Children children;

		struct State : WidgetState<ScrollView> {
			float scroll = 0.f;

			Child build(const Element &element) override {
				return Stack{
					.children{
						Scrollable{
							.children = widget->children,
						},
                        Scrollbar{
                            .scroll = scroll,
                        },
					},
				};
			}
		};
	};
}// namespace squi