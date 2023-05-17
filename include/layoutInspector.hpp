#ifndef SQUI_LAYOUTINSPECTOR_HPP
#define SQUI_LAYOUTINSPECTOR_HPP

#include "widget.hpp"
#include <functional>
#include <memory>

namespace squi {
    struct LayoutInspector {
        // Args
        std::vector<std::shared_ptr<Widget>> &content;
        std::vector<std::shared_ptr<Widget>> &overlays;
        
        struct Storage {
            // Data
			std::vector<std::shared_ptr<Widget>> &content;
			std::vector<std::shared_ptr<Widget>> &overlays;
            uint64_t selectedWidgetID = 0;
            uint64_t activeButtonID = 0;
            uint64_t hoveredWidgetID = 0;
            bool shouldUpdate = true;
            bool selectedWidgetChanged = false;
            bool pauseUpdates = false;
            bool pauseUpdatesChanged = false;
		};
    
        operator Child() const;
    };
}

#endif