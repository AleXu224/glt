#ifndef SQUI_LAYOUTINSPECTOR_HPP
#define SQUI_LAYOUTINSPECTOR_HPP

#include "widget.hpp"
#include <functional>
#include <memory>

namespace squi {
    struct LayoutInspector {
        // Args
        std::vector<Child> &content;
        std::vector<Child> &overlays;
        
        struct Storage {
            // Data
			std::vector<Child> &content;
			std::vector<Child> &overlays;
            ChildRef selectedWidget{};
            ChildRef activeButton{};
            ChildRef hoveredWidget{};
            bool shouldUpdate = true;
            bool selectedWidgetChanged = false;
            bool pauseUpdates = false;
            bool pauseUpdatesChanged = false;
		};
    
        operator Child() const;
    };
}

#endif