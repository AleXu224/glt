#include "scrollableFrame.hpp"
#include "scrollable.hpp"
#include "scrollbar.hpp"
#include "align.hpp"
#include "stack.hpp"

using namespace squi;

ScrollableFrame::operator squi::Child() const {
    auto storage = std::make_shared<Storage>();

    return Stack{
        .widget{widget},
        .children{
            Scrollable{
                .widget{
                    .width = Size::Expand,
                    .height = Size::Expand,
                },
                .onScroll = [storage](auto scroll, auto contentHeight, auto viewHeight) {
                    storage->scroll = scroll;
                    storage->contentHeight = contentHeight;
                    storage->viewHeight = viewHeight;
                },
                .setScroll = [storage]() {
                    return storage->scroll;
                },
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
                        .onScroll = [storage](const float &scroll) {
                            storage->scroll = scroll;
                        },
                        .setScroll = [storage]() {
                            return std::make_tuple(storage->scroll, storage->contentHeight, storage->viewHeight);
                        },
                    },
                },
            },
        },
    };
}