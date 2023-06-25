#include "navigationView.hpp"
#include "column.hpp"

using namespace squi;

struct NavigationMenu {
    // Args
    Widget::Args widget;

    struct Storage {
        // Data
    };

    operator Child() const {
        auto storage = std::make_shared<Storage>();

        return Column{
            .widget{widget},
            .children{
                
            },
        };
    }
};

NavigationView::operator Child() const {
	auto storage = std::make_shared<Storage>();

	return {};
}

