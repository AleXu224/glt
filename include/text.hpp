#ifndef SQUI_TEXT_HPP
#define SQUI_TEXT_HPP

#include "widget.hpp"
#include "color.hpp"
#include "quad.hpp"
#include <string_view>

namespace squi {
    struct Text {
        // Args
        Widget::Args widget;
        std::string text;
        float fontSize{14.0f};
        bool lineWrap{false};
        std::string fontPath{R"(C:\Windows\Fonts\arial.ttf)"};
        Color color{Color::HEX(0xFFFFFFFF)};
    
        class Impl : public Widget {
            // Data
            float lastX{0};
            float lastY{0};
            float lastParentWidth{0};
			std::string text;
			float fontSize;
            bool lineWrap;
            std::string fontPath;
			Color color;
            std::vector<std::vector<Quad>> quads{};

		public:
            Impl(const Text &args);

            void onUpdate() override;

            void onDraw() override;

            void setText(const std::string_view &text);

            [[nodiscard]] std::tuple<uint32_t, uint32_t> getTextSize(const std::string_view &text) const;

            [[nodiscard]] std::string_view getText() const;
        };
    
        operator Child() const {
            return {std::make_shared<Impl>(*this)};
        }
    };
}

#endif