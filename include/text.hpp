#ifndef SQUI_TEXT_HPP
#define SQUI_TEXT_HPP

#include "widget.hpp"
#include "color.hpp"
#include "quad.hpp"

namespace squi {
    struct Text {
        // Args
        Widget::Args widget;
        std::string text;
        float fontSize{14.0f};
        std::string fontPath{"C:\\Windows\\Fonts\\arial.ttf"};
        Color color{Color::HEX(0xFFFFFFFF)};
    
        class Impl : public Widget {
            // Data
            float lastX{0};
            float lastY{0};
			std::string text;
			float fontSize;
            std::string fontPath;
			Color color;
            std::vector<Quad> quads{};

		public:
            Impl(const Text &args);

            void onUpdate() override;

            void onDraw() override;
        };
    
        operator Child() const {
            return Child(std::make_shared<Impl>(*this));
        }
    };
}

#endif