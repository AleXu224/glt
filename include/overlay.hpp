#ifndef SQUI_OVERLAY_HPP
#define SQUI_OVERLAY_HPP

#include "child.hpp"
#include "widget.hpp"

namespace squi {
    struct Overlay {
        // Args
        Widget::Args widget;
        Child child{};
    
        class Impl : public Widget {
            // Data
            bool shouldBeDestroyed = false;
            bool canBeDestroyed = false;
    
        public:
            Impl(const Overlay &args);
        };
    
        operator Child() const;
    };
}

#endif