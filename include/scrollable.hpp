#ifndef SQUI_SCROLLABLE_HPP
#define SQUI_SCROLLABLE_HPP

#include "widget.hpp"
#include "column.hpp"

namespace squi {
    struct Scrollable {
        // Args
        Widget::Args widget;
        Column::Alignment alignment{Column::Alignment::left};
        float spacing{0.0f};
        Children children{};
    
        class Impl : public Widget {
        public:
            float scroll = 0;

            Impl(const Scrollable &args);

            void onUpdate() override;

            void onDraw() override;
        };
    
        operator Child() const {
            return Child(std::make_shared<Impl>(*this));
        }
    };
}

#endif