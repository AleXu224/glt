#pragma once

#include "widget.hpp"

namespace squi {
    struct Image {
        // Args
        Widget::Args widget;
    
        struct Storage {
            // Data
        };
    
        operator Child() const {
            auto storage = std::make_shared<Storage>();
    
            return {};
        }
    };
}