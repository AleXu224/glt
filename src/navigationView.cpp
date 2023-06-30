#include "navigationView.hpp"
#include "column.hpp"
#include "navigationMenu.hpp"
#include "row.hpp"

using namespace squi;

NavigationView::operator Child() const {
	auto storage = std::make_shared<Storage>();

	return Row{
        .children{
            NavigationMenu{},
        },
    };
}

