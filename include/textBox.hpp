#ifndef SQUI_TEXTBOX_HPP
#define SQUI_TEXTBOX_HPP

#include "color.hpp"
#include "gestureDetector.hpp"
#include "widget.hpp"
#include <memory>
#include <string_view>

namespace squi {
    struct TextBox {
        struct Theme {
			Color rest{Color::HEX(0xFFFFFF0F)};
			Color hover{Color::HEX(0xFFFFFF15)};
			Color active{Color::HEX(0x1E1E1EB2)};
			Color disabled{Color::HEX(0xFFFFFF0B)};

			Color border{Color::HEX(0xFFFFFF14)};
			Color borderActive{Color::HEX(0xFFFFFF12)};

			Color bottomBorder{Color::HEX(0xFFFFFF8B)};
			Color bottomBorderActive{Color::HEX(0x60CDFFFF)};

			Color text{Color::HEX(0xFFFFFFFF)};
			Color textDisabled{Color::HEX(0xFFFFFF5D)};
			Color textHint{Color::HEX(0xFFFFFFC8)};
		};
        static Theme theme;
        // Args
        Widget::Args widget;
    
        struct Storage {
            // Data
            enum class State {
                rest,
                hover,
                active,
                disabled,
            };
            State state = State::rest;
            bool changed = false;
            std::shared_ptr<GestureDetector::Storage> gd{};
        };
    
        operator Child() const;
    };
}

#endif