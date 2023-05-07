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
            std::shared_ptr<Widget> selectedWidget{};
            bool shouldUpdate = true;
            bool selectedWidgetChanged = false;
		};
    
        operator Child() const;
    };
}

#endif