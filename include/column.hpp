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
            explicit Impl(const Column &args);

            void onLayout(vec2 &maxSize, vec2 &minSize) override;
            void onArrange(vec2 &pos) override;

            float getMinHeight() override;

            void onDraw() override;
        };
    
        operator Child() const {
            return {std::make_shared<Impl>(*this)};
        }
    };
}

#endif