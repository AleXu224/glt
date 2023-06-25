#ifndef SQUI_NAVIGATIONMENU_HPP
#define SQUI_NAVIGATIONMENU_HPP

#include "widget.hpp"

namespace squi {
    struct WidgetName {
        // Args
        Widget::Args widget;
    
        struct Storage {
            // Data
        };
    
        // operator Child() const {
        //     auto storage = std::make_shared<Storage>();
    
        //     return Child(std::make_shared<Widget>(this->widget));
        // }
    };
}

#endif