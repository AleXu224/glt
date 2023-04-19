#ifndef SQUI_CONTEXTMENU_HPP
#define SQUI_CONTEXTMENU_HPP

#include "widget.hpp"

namespace squi {
    struct ContextMenu {
        // Args
        Widget::Args widget;
    
        struct Storage {
            // Data
        };
    
        operator Child() const;
    };
}

#endif