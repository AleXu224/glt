#ifndef SQUI_TEXTINPUT_HPP
#define SQUI_TEXTINPUT_HPP

#include "color.hpp"
#include "gestureDetector.hpp"
#include "widget.hpp"
#include <memory>
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
            float scroll{};
            std::optional<uint32_t> selectionStart{};

		public:
            explicit Impl(const TextInput &args);

            void onUpdate() override;
            void onLayout(vec2 &maxSize, vec2 &minSize) override;
            void onArrange(vec2 &pos) override;

            void setActive(bool active);

            void onDraw() override;
        };
    
        operator Child() const;
    };
}

#endif