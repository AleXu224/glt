#ifndef SQUI_STACK_HPP
#define SQUI_STACK_HPP

#include "widget.hpp"

namespace squi {
    struct Stack {
        // Args
        Widget::Args widget;
        Children children{};
    
        class Impl : public Widget {
            // Data
    
        public:
            Impl(const Stack &args);
        };
    
        operator Child() const {
            return Child(std::make_shared<Impl>(*this));
        }
    };
}

#endif