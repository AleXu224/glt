#ifndef SQUI_TEXTBOX_HPP
#define SQUI_TEXTBOX_HPP

#include "widget.hpp"
#include <string_view>

namespace squi {
    struct TextBox {
        // Args
        Widget::Args widget;
    
        struct Storage {
            // Data
        };
    
        operator Child() const;
    };
}

#endif