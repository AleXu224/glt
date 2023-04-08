#ifndef SQUI_COLUMN_HPP
#define SQUI_COLUMN_HPP

#include "widget.hpp"

namespace squi {
    struct Column {
        // Helpers
        enum class Alignment {
            left,
            center,
            right,
        };
        
        // Args
        Widget::Args widget;
        Alignment alignment{Alignment::left};
        float spacing{0.0f};
        Children children{};
    
        class Impl : public Widget {
            // Data
            Alignment alignment;
            float spacing;
    
        public:
            Impl(const Column &args);

            void matchChildSizeBehavior(bool, bool) override;

            void onUpdate() override;

            void onDraw() override;
        };
    
        operator std::shared_ptr<Widget>() const {
            return std::make_shared<Impl>(*this);
        }
    
        operator Child() const {
            return Child(std::make_shared<Impl>(*this));
        }
    };
}

#endif