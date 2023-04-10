#ifndef SQUI_SCROLLBAR_HPP
#define SQUI_SCROLLBAR_HPP

#include "widget.hpp"
#include "memory"
#include "box.hpp"

namespace squi {
    struct Scrollbar {
        // Args
        Widget::Args widget;
        std::function<void(Widget &)> onScroll{};
    
        struct Storage {
            // Data
            float scroll = 0;
            float scrollDragStart = 0;
        };
    
        operator Child() const;
    };
}

#endif