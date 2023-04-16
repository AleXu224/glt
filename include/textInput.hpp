#ifndef SQUI_TEXTINPUT_HPP
#define SQUI_TEXTINPUT_HPP

#include "color.hpp"
#include "widget.hpp"
#include <optional>
#include <string_view>

namespace squi {
    struct TextInput {
        // Args
        Widget::Args widget;
        float fontSize = 14.0f;
        std::string_view font = R"(C:\Windows\Fonts\arial.ttf)";
        Color color{Color::HEX(0xFFFFFFFF)};
    
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