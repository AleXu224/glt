#pragma once
#include "child.hpp"
#include "widget.hpp"
#include <vector>

namespace squi {
    struct Navigator {
        struct Controller {
            void push(const Child& child);
            void pop();
        private:
            friend Navigator;
            std::vector<Child> pushStack{};
            uint32_t popCount = 0;
        };
        
        // Args
        Widget::Args widget;
        Controller controller;
        Child child;
    
        class Impl : public Widget {
            // Data
            Controller controller;
    
        public:
            Impl(const Navigator &args);
        };
    
        operator Child() const {
            return std::make_shared<Impl>(*this);
        }
    };
}