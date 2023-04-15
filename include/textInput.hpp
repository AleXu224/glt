#ifndef SQUI_TEXTINPUT_HPP
#define SQUI_TEXTINPUT_HPP

#include "widget.hpp"
#include <optional>

namespace squi {
    struct TextInput {
        // Args
        Widget::Args widget;
    
        class Impl : public Widget {
            // Data
            uint32_t cursor{};
            float startToCursor{};
            float startToSelection{};
            float selectionStartToSelectionEnd{};
            float scroll{};
            std::optional<uint32_t> selectionStart{};
    
        public:
            explicit Impl(const TextInput &args);

            void onUpdate() override;

            void onDraw() override;
        };
    
        operator Child() const {
            return {std::make_shared<Impl>(*this)};
        }
    };
}

#endif