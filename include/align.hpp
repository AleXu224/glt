#ifndef SQUI_ALIGN_HPP
#define SQUI_ALIGN_HPP

#include "widget.hpp"

namespace squi {
    struct Align {
        // Args
		/**
         * The x alignment of the child.
         * 0.0f is left, 1.0f is right, 0.5f is center.
         */
		float xAlign{0.5f};
		/**
         * The y alignment of the child.
         * 0.0f is top, 1.0f is bottom, 0.5f is center.
         */
		float yAlign{0.5f};
		Child child{};

		struct Storage {
            // Data
            float xAlign;
            float yAlign;
        };
    
        operator Child() const;
    };
}

#endif