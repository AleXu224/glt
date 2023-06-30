#ifndef SQUI_NAVIGATIONMENU_HPP
#define SQUI_NAVIGATIONMENU_HPP

#include "widget.hpp"

namespace squi {
    struct NavigationMenu {
        // Args
        Widget::Args widget;
    
        struct Storage {
            // Data
        };
    
        operator Child() const;
    };
}

#endif