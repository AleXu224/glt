#ifndef SQUI_SCROLLABLE_HPP
#define SQUI_SCROLLABLE_HPP

#include "gestureDetector.hpp"
#include "widget.hpp"
#include "column.hpp"
#include <memory>

namespace squi {
    struct Scrollable {
        // Args
        Widget::Args widget;
        Column::Alignment alignment{Column::Alignment::left};
        float spacing{0.0f};
        /**
         * @brief onScroll(float scroll, float contentHeight, float viewHeight)
         */
        std::function<void(float, float, float)> onScroll{};
        std::function<float()> setScroll{};
        Children children{};
    
        class Impl : public Widget {
        public:
            float scroll = 0;
            bool scrolled = false;

            Impl(const Scrollable &args);

            void onUpdate() final;
            void afterUpdate() final;
            void arrangeChildren(vec2 &pos) final;

            void drawChildren() final;
        };
    
        operator Child() const;
    };
}

#endif