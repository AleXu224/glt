#ifndef SQUI_LAYOUTINSPECTOR_HPP
#define SQUI_LAYOUTINSPECTOR_HPP

#include "widget.hpp"

namespace squi {
    struct LayoutInspector {
        // Args
        Widget* window;
        
        struct Storage {
            // Data
        };
    
        operator Child() const;
    };
}

#endif