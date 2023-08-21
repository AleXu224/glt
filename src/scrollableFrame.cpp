#include "scrollableFrame.hpp"
#include "align.hpp"
#include "scrollable.hpp"
#include "scrollbar.hpp"
#include "stack.hpp"


using namespace squi;

ScrollableFrame::operator squi::Child() const {
	auto storage = std::make_shared<Storage>();
	auto controller = std::make_shared<Scrollable::Controller>();

	return Stack{
		.widget{widget},
		.children{
			Scrollable{
				.widget{scrollableWidget},
				.alignment = alignment,
				.spacing = spacing,
				.onScroll = [storage](auto scroll, auto contentHeight, auto viewHeight) {
					storage->scroll = scroll;
					storage->contentHeight = contentHeight;
					storage->viewHeight = viewHeight;
				},
				.controller = controller,
				.children{children},
			},
			Align{
				.xAlign = 1,
				.yAlign = 0,
				.child{
					Scrollbar{
						.widget{
							.height = Size::Expand,
						},
						.controller = controller,
					},
				},
			},
		},
	};
}