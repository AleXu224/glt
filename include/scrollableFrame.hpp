#ifndef SQUI_SCROLLABLEFRAME_HPP
#define SQUI_SCROLLABLEFRAME_HPP

#include "widget.hpp"

namespace squi {
    struct ScrollableFrame {
        // Args
        Widget::Args widget;
        Children children;
    
        struct Storage {
            // Data
            float contentHeight = 200;
            float viewHeight = 100;
            float scroll = 0;
        };
    
        operator Child() const;
    };
}

#endif